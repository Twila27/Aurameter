#include "Engine/Renderer/OpenGLExtensions.hpp"
//#pragma comment( lib, "opengl32" ) // Link in the OpenGL32.lib static library

//-----------------------------------------------------------------------------

// Globals "function pointer" variables (these go in OpenGLExtensions.cpp or similar)
PFNGLGENBUFFERSPROC			glGenBuffers		= nullptr;
PFNGLBINDBUFFERPROC			glBindBuffer		= nullptr;
PFNGLBUFFERDATAPROC			glBufferData		= nullptr;
PFNGLDELETEBUFFERSPROC		glDeleteBuffers		= nullptr;


//-----------------------------------------------------------------------------

//Loading a shader.
PFNGLCREATESHADERPROC		glCreateShader = nullptr;
PFNGLSHADERSOURCEPROC		glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC		glCompileShader = nullptr;
PFNGLGETSHADERIVPROC		glGetShaderiv = nullptr;
PFNGLDELETESHADERPROC		glDeleteShader = nullptr;


//-----------------------------------------------------------------------------

//Loading a program.
PFNGLCREATEPROGRAMPROC	glCreateProgram		= nullptr;
PFNGLATTACHSHADERPROC	glAttachShader		= nullptr;
PFNGLLINKPROGRAMPROC	glLinkProgram		= nullptr;
PFNGLGETPROGRAMIVPROC	glGetProgramiv		= nullptr;
PFNGLDETACHSHADERPROC	glDetachShader		= nullptr;
PFNGLDELETEPROGRAMPROC	glDeleteProgram		= nullptr;


//-----------------------------------------------------------------------------

//Error checking.
PFNGLGETSHADERINFOLOGPROC	glGetShaderInfoLog = nullptr;
PFNGLGETPROGRAMINFOLOGPROC	glGetProgramInfoLog	= nullptr;


//-----------------------------------------------------------------------------

//Vertex array descriptor objects.
PFNGLGENVERTEXARRAYSPROC			glGenVertexArrays			= nullptr;
PFNGLDELETEVERTEXARRAYSPROC			glDeleteVertexArrays		= nullptr;
PFNGLBINDVERTEXARRAYPROC			glBindVertexArray			= nullptr;
PFNGLGETATTRIBLOCATIONPROC			glGetAttribLocation = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC	glEnableVertexAttribArray = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC	glDisableVertexAttribArray = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC		glVertexAttribPointer		= nullptr;
PFNGLVERTEXATTRIBIPOINTERPROC		glVertexAttribIPointer		= nullptr;



//-----------------------------------------------------------------------------

//Drawing. Already have glDrawArrays.
PFNGLUSEPROGRAMPROC					glUseProgram				= nullptr;


//-----------------------------------------------------------------------------

//Uniforms.
PFNGLGETUNIFORMLOCATIONPROC		glGetUniformLocation = nullptr;
PFNGLGETACTIVEUNIFORMPROC		glGetActiveUniform = nullptr;

PFNGLUNIFORM1FVPROC				glUniform1fv = nullptr;
PFNGLUNIFORM2FVPROC				glUniform2fv = nullptr;
PFNGLUNIFORM3FVPROC				glUniform3fv = nullptr;
PFNGLUNIFORM4FVPROC				glUniform4fv = nullptr;

PFNGLUNIFORM1IVPROC				glUniform1iv = nullptr;
PFNGLUNIFORM2IVPROC				glUniform2iv = nullptr;
PFNGLUNIFORM3IVPROC				glUniform3iv = nullptr;
PFNGLUNIFORM4IVPROC				glUniform4iv = nullptr;

PFNGLUNIFORMMATRIX4FVPROC		glUniformMatrix4fv = nullptr;


//-----------------------------------------------------------------------------

//Samplers and textures.
PFNGLGENSAMPLERSPROC			glGenSamplers = nullptr;
PFNGLSAMPLERPARAMETERIPROC		glSamplerParameteri = nullptr;
PFNGLBINDSAMPLERPROC			glBindSampler = nullptr;
PFNGLACTIVETEXTUREPROC			glActiveTexture = nullptr;
PFNGLDELETESAMPLERSPROC			glDeleteSamplers = nullptr;


//-----------------------------------------------------------------------------

//Frame buffer objects.
PFNGLGENFRAMEBUFFERSPROC			glGenFramebuffers = nullptr;
PFNGLBINDFRAMEBUFFERPROC			glBindFramebuffer = nullptr;
PFNGLFRAMEBUFFERTEXTUREPROC			glFramebufferTexture = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSPROC		glCheckFramebufferStatus = nullptr;
PFNGLDELETEFRAMEBUFFERSPROC			glDeleteFramebuffers = nullptr;
PFNGLDRAWBUFFERSPROC				glDrawBuffers = nullptr;
PFNGLBLITFRAMEBUFFERPROC			glBlitFramebuffer = nullptr;


//--------------------------------------------------------------------------------------------------------------

//Needed for RenderDoc to capture.
PFNWGLCREATECONTEXTATTRIBSARBPROC	wglCreateContextAttribsARB = nullptr;

//Needed for RiftUtils.hpp.
PFNGLGENERATEMIPMAPPROC				glGenerateMipmap = nullptr;
PFNGLFRAMEBUFFERTEXTURE2DPROC		glFramebufferTexture2D = nullptr;
PFNGLFRAMEBUFFERRENDERBUFFERPROC	glFramebufferRenderbuffer = nullptr;
