#include "Engine/Renderer/FrameBuffer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/OpenGLExtensions.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/String/StringUtils.hpp"


//--------------------------------------------------------------------------------------------------------------
FrameBuffer::FrameBuffer( size_t numColorRenderTargets, Texture** colorRenderTargets, Texture* depthStencilTarget /*= nullptr */ )
{
	//Actual creation.
	glGenFramebuffers( 1, &m_fboID );
	ASSERT_OR_DIE( m_fboID != NULL, "m_fboID == NULL in FrameBuffer()" );

	//-----------------------------------------------------------------------------

	ASSERT_OR_DIE( numColorRenderTargets > 0, "NumColorRenderTargets > 0 in FrameBuffer()" );
	//You CAN have an FBO that just draws to the depth, but this makes below easier to write.

	Texture* color0 = colorRenderTargets[ 0 ];
	unsigned int standardWidth = color0->GetWidth();
	unsigned int standardHeight = color0->GetHeight();

	//Everything we bind needs to match widths and heights.
	for ( unsigned int i = 1; i < numColorRenderTargets; i++ )
	{
		Texture* color = colorRenderTargets[ i ];
		ASSERT_OR_DIE( ( color->GetWidth() == standardWidth ) && ( color->GetHeight() == standardHeight ), "Mismatched Color Render Target Dims in FrameBuffer()" );
		//Can return null instead if data's coming in from outside.
		//But mostly this is here to protect you from you at 3AM.
	}

	if ( depthStencilTarget != nullptr )
	{
		ASSERT_OR_DIE( ( depthStencilTarget->GetWidth() == standardWidth ) && ( depthStencilTarget->GetHeight() == standardHeight ),
					   "Mismatched Depth/Stencil Render Target Dims in FrameBuffer()" );
	}

	//Add all textures to it.
	for ( unsigned int i = 0; i < numColorRenderTargets; i++ )
	{
		m_colorRenderTargets.push_back( colorRenderTargets[ i ] );
	}
	m_depthStencilRenderTarget = depthStencilTarget;

	//-----------------------------------------------------------------------------

	// OpenGL Init: if you bound an FBO to your Renderer, be careful this below didn't just cause that to be unexpectedly unbound!
	glBindFramebuffer( GL_FRAMEBUFFER, m_fboID );

	for ( unsigned int renderTargetIndex = 0; renderTargetIndex < numColorRenderTargets; renderTargetIndex++ )
	{
		Texture* tex = colorRenderTargets[ renderTargetIndex ];
		glFramebufferTexture( GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0 + renderTargetIndex, //Attachment point, i.e. what we are attaching to GL_FRAMEBUFFER.
			tex->GetTextureID(),
			0 // level - probably mipmap level, we don't care much.
		);
	}

	//Bind depth stencil if I have it.
	if ( depthStencilTarget != nullptr )
	{
		glFramebufferTexture( GL_FRAMEBUFFER,
			GL_DEPTH_STENCIL_ATTACHMENT, //Attachment point, i.e. what we are attaching to GL_FRAMEBUFFER.
			depthStencilTarget->GetTextureID(),
			0 // level - probably mipmap level, we don't care much.
		);
	}

	//Validation.
	GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	if ( status != GL_FRAMEBUFFER_COMPLETE )
		ERROR_RECOVERABLE( "glCheckFrameBufferStatus incomplete in FrameBuffer()" );

	//Revert to old state via unbind.
	glBindFramebuffer( GL_FRAMEBUFFER, NULL );

	m_pixelWidth = standardWidth;
	m_pixelHeight = standardHeight;
}


//--------------------------------------------------------------------------------------------------------------
FrameBuffer::FrameBuffer( unsigned int width, unsigned int height, const std::vector<TextureFormat>& colorFormats, const TextureFormat* depthStencilFormat /*=nullptr*/ )
	: m_pixelWidth( width )
	, m_pixelHeight( height )
	, m_fboID( NULL )
{	
	//Actual creation.
	glGenFramebuffers( 1, &m_fboID );
	ASSERT_OR_DIE( m_fboID != NULL, "m_fboID == NULL in FrameBuffer()" );

	//Generate the texture render targets managed by and associated with this FrameBuffer.
	unsigned int numColorRenderTargets = colorFormats.size();

	Texture** colorRenderTargets = new Texture*[ numColorRenderTargets ];
	for ( unsigned int formatIndex = 0; formatIndex < numColorRenderTargets; formatIndex++ )
	{
		const TextureFormat& currentFormat = colorFormats[ formatIndex ];
		colorRenderTargets[ formatIndex ] = Texture::CreateTextureFromNoData( Stringf( "ColorTarget%d_FBO%d", formatIndex, m_fboID ), width, height, currentFormat );
	}

	Texture* depthStencilTarget = nullptr;
	if ( depthStencilFormat != nullptr )
	{
		//Make that tex.
		depthStencilTarget = Texture::CreateTextureFromNoData( "DepthStencilTarget_FBO" + m_fboID, width, height, *depthStencilFormat );
	}

	//-----------------------------------------------------------------------------

	ASSERT_OR_DIE( numColorRenderTargets > 0, "NumColorRenderTargets > 0 in FrameBuffer()" );
	//You CAN have an FBO that just draws to the depth, but this makes below easier to write.

	Texture* color0 = colorRenderTargets[ 0 ];
	unsigned int standardWidth = color0->GetWidth();
	unsigned int standardHeight = color0->GetHeight();

	//Everything we bind needs to match widths and heights.
	for ( unsigned int i = 1; i < numColorRenderTargets; i++ )
	{
		Texture* color = colorRenderTargets[ i ];
		ASSERT_OR_DIE( ( color->GetWidth() == standardWidth ) && ( color->GetHeight() == standardHeight ), "Mismatched Color Render Target Dims in FrameBuffer()" );
		//Can return null instead if data's coming in from outside.
		//But mostly this is here to protect you from you at 3AM.
	}

	if ( depthStencilTarget != nullptr )
	{
		ASSERT_OR_DIE( ( depthStencilTarget->GetWidth() == standardWidth ) && ( depthStencilTarget->GetHeight() == standardHeight ),
					   "Mismatched Depth/Stencil Render Target Dims in FrameBuffer()" );
	}

	//Add all textures to it.
	for ( unsigned int i = 0; i < numColorRenderTargets; i++ )
	{
		m_colorRenderTargets.push_back( colorRenderTargets[ i ] );
	}
	m_depthStencilRenderTarget = depthStencilTarget;

	//-----------------------------------------------------------------------------

	// OpenGL Init: if you bound an FBO to your Renderer, be careful this below didn't just cause that to be unexpectedly unbound!
	glBindFramebuffer( GL_FRAMEBUFFER, m_fboID );

	for ( unsigned int renderTargetIndex = 0; renderTargetIndex < numColorRenderTargets; renderTargetIndex++ )
	{
		Texture* tex = colorRenderTargets[ renderTargetIndex ];
		glFramebufferTexture( GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0 + renderTargetIndex, //Attachment point, i.e. what we are attaching to GL_FRAMEBUFFER.
			tex->GetTextureID(),
			0 // level - probably mipmap level, we don't care much.
		);
	}

	//Bind depth stencil if I have it.
	if ( depthStencilTarget != nullptr )
	{
		glFramebufferTexture( GL_FRAMEBUFFER,
			GL_DEPTH_STENCIL_ATTACHMENT, //Attachment point, i.e. what we are attaching to GL_FRAMEBUFFER.
			depthStencilTarget->GetTextureID(),
			0 // level - probably mipmap level, we don't care much.
		);
	}

	//Validation.
	GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	if ( status != GL_FRAMEBUFFER_COMPLETE )
		ERROR_RECOVERABLE( "glCheckFrameBufferStatus incomplete in FrameBuffer()" );

	//Revert to old state via unbind.
	glBindFramebuffer( GL_FRAMEBUFFER, NULL );

	if ( colorRenderTargets != nullptr )
		delete colorRenderTargets;

	m_pixelWidth = standardWidth;
	m_pixelHeight = standardHeight;
}


//--------------------------------------------------------------------------------------------------------------
FrameBuffer::~FrameBuffer()
{
	//Cleaned up by TheRenderer::DeleteTextures.
	if ( m_colorRenderTargets.size() > 0 )
	{
		for ( Texture* tex : m_colorRenderTargets )
		{
			if ( tex != nullptr )
			{
//				delete tex;
				tex = nullptr;
			}
		}
	}

	if ( m_depthStencilRenderTarget != nullptr )
	{
//		delete m_depthStencilRenderTarget;
		m_depthStencilRenderTarget = nullptr;
	}

//	glDeleteFramebuffers( 1, &m_fboID );
}
