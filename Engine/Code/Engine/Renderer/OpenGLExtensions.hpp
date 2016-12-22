#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLu.h>
#include "Engine/Renderer/glext.h"
#include "Engine/Renderer/wglext.h"


//Engine/Renderer/TheRenderer.cpp::TheRenderer() initializes the below declarations.

//--------------------------------------------------------------------------------------------------------------

//Include this header by TheRenderer.
extern PFNGLGENBUFFERSPROC				glGenBuffers;
extern PFNGLBINDBUFFERPROC				glBindBuffer;
extern PFNGLBUFFERDATAPROC				glBufferData;
extern PFNGLDELETEBUFFERSPROC			glDeleteBuffers;

//--------------------------------------------------------------------------------------------------------------

//Loading a shader.
extern PFNGLCREATESHADERPROC			glCreateShader;
extern PFNGLSHADERSOURCEPROC			glShaderSource;
extern PFNGLCOMPILESHADERPROC			glCompileShader;
extern PFNGLGETSHADERIVPROC				glGetShaderiv;
extern PFNGLDELETESHADERPROC			glDeleteShader;


//--------------------------------------------------------------------------------------------------------------

//Loading a program.
extern PFNGLCREATEPROGRAMPROC			glCreateProgram;
extern PFNGLATTACHSHADERPROC			glAttachShader;
extern PFNGLLINKPROGRAMPROC				glLinkProgram;
extern PFNGLGETPROGRAMIVPROC			glGetProgramiv;
extern PFNGLDETACHSHADERPROC			glDetachShader;
extern PFNGLDELETEPROGRAMPROC			glDeleteProgram;


//--------------------------------------------------------------------------------------------------------------

//Error checking.
extern PFNGLGETSHADERINFOLOGPROC		glGetShaderInfoLog;
extern PFNGLGETPROGRAMINFOLOGPROC		glGetProgramInfoLog;


//--------------------------------------------------------------------------------------------------------------

//Vertex array descriptor objects.
extern PFNGLGENVERTEXARRAYSPROC			glGenVertexArrays;
extern PFNGLDELETEVERTEXARRAYSPROC		glDeleteVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC			glBindVertexArray;
extern PFNGLGETATTRIBLOCATIONPROC		glGetAttribLocation;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC	glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC		glVertexAttribPointer;
extern PFNGLVERTEXATTRIBIPOINTERPROC	glVertexAttribIPointer;


//--------------------------------------------------------------------------------------------------------------

//Drawing. glDrawArrays we link in with the core.
extern PFNGLUSEPROGRAMPROC				glUseProgram;


//--------------------------------------------------------------------------------------------------------------

//Uniforms.
extern PFNGLGETUNIFORMLOCATIONPROC		glGetUniformLocation;
extern PFNGLGETACTIVEUNIFORMPROC		glGetActiveUniform;

extern PFNGLUNIFORM1FVPROC				glUniform1fv;
extern PFNGLUNIFORM2FVPROC				glUniform2fv;
extern PFNGLUNIFORM3FVPROC				glUniform3fv;
extern PFNGLUNIFORM4FVPROC				glUniform4fv;

extern PFNGLUNIFORM1IVPROC				glUniform1iv;
extern PFNGLUNIFORM2IVPROC				glUniform2iv;
extern PFNGLUNIFORM3IVPROC				glUniform3iv;
extern PFNGLUNIFORM4IVPROC				glUniform4iv;

extern PFNGLUNIFORMMATRIX4FVPROC		glUniformMatrix4fv;


//--------------------------------------------------------------------------------------------------------------

//Samplers and textures.
extern PFNGLGENSAMPLERSPROC				glGenSamplers;
extern PFNGLSAMPLERPARAMETERIPROC		glSamplerParameteri;
extern PFNGLBINDSAMPLERPROC				glBindSampler;
extern PFNGLACTIVETEXTUREPROC			glActiveTexture;
extern PFNGLDELETESAMPLERSPROC			glDeleteSamplers;


//--------------------------------------------------------------------------------------------------------------

//Frame buffer objects.
extern PFNGLGENFRAMEBUFFERSPROC			glGenFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC			glBindFramebuffer;
extern PFNGLFRAMEBUFFERTEXTUREPROC		glFramebufferTexture;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC	glCheckFramebufferStatus;
extern PFNGLDELETEFRAMEBUFFERSPROC		glDeleteFramebuffers;
extern PFNGLDRAWBUFFERSPROC				glDrawBuffers;
extern PFNGLBLITFRAMEBUFFERPROC			glBlitFramebuffer;


//--------------------------------------------------------------------------------------------------------------

//Needed to capture using RenderDoc.
extern PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;

//Needed for RiftUtils.hpp.
extern PFNGLGENERATEMIPMAPPROC			glGenerateMipmap;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC	glFramebufferTexture2D;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC	glFramebufferRenderbuffer;