#pragma once

#include "Engine/BuildConfig.hpp"

#ifdef PLATFORM_RIFT_CV1

#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Matrix4x4.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#include "OpenGLExtensions.hpp"

#include "OVR_CAPI_GL.h" //OpenGL structures used by the below code.
#include "Extras/OVR_MATH.h" //Needed for Sizei below, etc.

#if defined(_WIN32)
	#include <dxgi.h> //for GetDefaultAdapterLuid, comes with Windows/shared headers.
	#pragma comment( lib, "dxgi.lib")
#endif


//---------------------------------------------------------------------------------------
struct RiftCamera
{
	Vector3f eyeWorldPosition[2]; //Different to include IPD offsets per eye.
	Matrix4x4f eyeOrientation[2]; //I thought Oculus recommended this being the same for each eye, so why [2]?
	Matrix4x4f eyeViewTransform[2]; //Rotates around the axes in my basis.
};


//---------------------------------------------------------------------------------------
static Matrix4x4f GetPerspectiveRiftProjectionTransform();
static Matrix4x4f GetOrthographicRiftProjectionTransform();



//---------------------------------------------------------------------------------------
struct RiftDepthBuffer;
struct RiftTextureBuffer;
struct RiftRenderContext
{
	int currentEye; //0 or 1.

	RiftTextureBuffer* eyeRenderTexture[2] = { nullptr, nullptr };
	RiftDepthBuffer* eyeDepthBuffer[2] = { nullptr, nullptr };
	ovrMirrorTexture mirrorTexture = nullptr;
	GLuint mirrorFBO = 0;
	bool isVisible = true;
	long long frameIndex = 0;

	ovrSession session;
	ovrGraphicsLuid luid;
	ovrHmdDesc hmdDesc;
	double sensorSampleTime; //Set pre-render, then fed into a layer post-render.

	ovrEyeRenderDesc eyeRenderDesc[2];
	ovrPosef EyeRenderPose[2];

	RiftCamera camera;
};


//---------------------------------------------------------------------------------------
static ovrGraphicsLuid GetDefaultAdapterLuid()
{
	ovrGraphicsLuid luid = ovrGraphicsLuid();

#if defined(_WIN32)
	IDXGIFactory* factory = nullptr;

	if (SUCCEEDED(CreateDXGIFactory(IID_PPV_ARGS(&factory))))
	{
		IDXGIAdapter* adapter = nullptr;

		if (SUCCEEDED(factory->EnumAdapters(0, &adapter)))
		{
			DXGI_ADAPTER_DESC desc;

			adapter->GetDesc(&desc);
			memcpy(&luid, &desc.AdapterLuid, sizeof(luid));
			adapter->Release();
		}

		factory->Release();
	}

	return luid;
#endif
}

//---------------------------------------------------------------------------------------
struct RiftDepthBuffer
{
	GLuint        texId;

	RiftDepthBuffer(OVR::Sizei size, int sampleCount)
	{
		ASSERT_OR_DIE(sampleCount <= 1, "The code doesn't currently handle MSAA textures.");

		glGenTextures(1, &texId);
		glBindTexture(GL_TEXTURE_2D, texId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		GLenum internalFormat = GL_DEPTH_COMPONENT24;
		GLenum type = GL_UNSIGNED_INT;
		if ( true /*GLE_ARB_depth_buffer_float*/ ) //Defined true in the demo.
		{
			internalFormat = GL_DEPTH_COMPONENT32F;
			type = GL_FLOAT;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, size.w, size.h, 0, GL_DEPTH_COMPONENT, type, NULL);
	}
	~RiftDepthBuffer()
	{
		if (texId)
		{
			glDeleteTextures(1, &texId);
			texId = 0;
		}
	}
};


//--------------------------------------------------------------------------------------------------------------
struct RiftTextureBuffer
{
	ovrSession          Session;
	ovrTextureSwapChain TextureChain;
	GLuint				texId;
	GLuint				fboId;
	OVR::Sizei          texSize;

	RiftTextureBuffer(ovrSession session, bool rendertarget, bool displayableOnHmd, OVR::Sizei size, int mipLevels, unsigned char * data, int sampleCount) :
		Session(session),
		TextureChain(nullptr),
		texId(0),
		fboId(0),
		texSize(0, 0)
	{
		ASSERT_OR_DIE(sampleCount <= 1, "The Oculus demo code doesn't currently handle MSAA textures.");

		texSize = size;

		if (displayableOnHmd)
		{
			// This texture isn't necessarily going to be a rendertarget, but it usually is.
			ASSERT_OR_DIE(session, "No HMD? A little odd.");
			ASSERT_OR_DIE(sampleCount == 1, "ovr_CreateSwapTextureSetD3D11 doesn't support MSAA.");

			ovrTextureSwapChainDesc desc = {};
			desc.Type = ovrTexture_2D;
			desc.ArraySize = 1;
			desc.Width = size.w;
			desc.Height = size.h;
			desc.MipLevels = 1;
			desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
			desc.SampleCount = 1;
			desc.StaticImage = ovrFalse;

			ovrResult result = ovr_CreateTextureSwapChainGL(Session, &desc, &TextureChain);

			int length = 0;
			ovr_GetTextureSwapChainLength(session, TextureChain, &length);

			if (OVR_SUCCESS(result))
			{
				for (int i = 0; i < length; ++i)
				{
					GLuint chainTexId;
					ovr_GetTextureSwapChainBufferGL(Session, TextureChain, i, &chainTexId);
					glBindTexture(GL_TEXTURE_2D, chainTexId);

					if (rendertarget)
					{
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					}
					else
					{
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
					}
				}
			}
		}
		else
		{
			glGenTextures(1, &texId);
			glBindTexture(GL_TEXTURE_2D, texId);

			if (rendertarget)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}

			glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, texSize.w, texSize.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}

		if (mipLevels > 1)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		glGenFramebuffers(1, &fboId);
	}

	~RiftTextureBuffer()
	{
		if (TextureChain)
		{
			ovr_DestroyTextureSwapChain(Session, TextureChain);
			TextureChain = nullptr;
		}
		if (texId)
		{
			glDeleteTextures(1, &texId);
			texId = 0;
		}
		if (fboId)
		{
			glDeleteFramebuffers(1, &fboId);
			fboId = 0;
		}
	}

	OVR::Sizei GetSize() const
	{
		return texSize;
	}

	void SetAndClearRenderSurface(RiftDepthBuffer* dbuffer)
	{
		GLuint curTexId;
		if (TextureChain)
		{
			int curIndex;
			ovr_GetTextureSwapChainCurrentIndex(Session, TextureChain, &curIndex);
			ovr_GetTextureSwapChainBufferGL(Session, TextureChain, curIndex, &curTexId);
		}
		else
		{
			curTexId = texId;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, fboId);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dbuffer->texId, 0);

		glViewport(0, 0, texSize.w, texSize.h);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_FRAMEBUFFER_SRGB);
	}


	void SetRenderSurface( RiftDepthBuffer* dbuffer )
	{
		GLuint curTexId;
		if ( TextureChain )
		{
			int curIndex;
			ovr_GetTextureSwapChainCurrentIndex( Session, TextureChain, &curIndex );
			ovr_GetTextureSwapChainBufferGL( Session, TextureChain, curIndex, &curTexId );
		}
		else
		{
			curTexId = texId;
		}

		glBindFramebuffer( GL_FRAMEBUFFER, fboId );
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0 );
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dbuffer->texId, 0 );

		glViewport( 0, 0, texSize.w, texSize.h );
//		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glEnable( GL_FRAMEBUFFER_SRGB );
	}

	void UnsetRenderSurface()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fboId);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
	}

	void Commit()
	{
		if (TextureChain)
		{
			ovr_CommitTextureSwapChain(Session, TextureChain);
		}
	}
};

#endif