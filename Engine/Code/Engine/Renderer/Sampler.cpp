#include "Engine/Renderer/Sampler.hpp"


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#include "Engine/Renderer/OpenGLExtensions.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"

//--------------------------------------------------------------------------------------------------------------
Sampler::Sampler()
{
	m_samplerOptions.m_magFilter = FILTER_NEAREST;
	m_samplerOptions.m_minFilter = FILTER_NEAREST;
	m_samplerOptions.m_uWrap = WRAP_CLAMP_TO_EDGE;
	m_samplerOptions.m_vWrap = WRAP_CLAMP_TO_EDGE;

	m_openglSamplerID = RegisterSampler( FILTER_NEAREST, FILTER_NEAREST, WRAP_CLAMP_TO_EDGE, WRAP_CLAMP_TO_EDGE );
}


//--------------------------------------------------------------------------------------------------------------
Sampler::Sampler( SamplerFilter minFilter, SamplerFilter magFilter, SamplerWrapMode uWrapMode, SamplerWrapMode vWrapMode )
{
	m_samplerOptions.m_magFilter = magFilter;
	m_samplerOptions.m_minFilter = minFilter;
	m_samplerOptions.m_uWrap = uWrapMode;
	m_samplerOptions.m_vWrap = vWrapMode;

	m_openglSamplerID = RegisterSampler( minFilter, magFilter, uWrapMode, vWrapMode );
}


//--------------------------------------------------------------------------------------------------------------
Sampler::Sampler( const SamplerOptions* options )
{
	if ( options == nullptr ) return;

	Sampler( options->m_magFilter, options->m_minFilter, options->m_uWrap, options->m_vWrap );
}


//--------------------------------------------------------------------------------------------------------------
unsigned int GetOpenGLSamplerFilter( SamplerFilter engineSampleFilter )
{
	switch ( engineSampleFilter )
	{
	case SamplerFilter::FILTER_LINEAR: return GL_LINEAR;
	case SamplerFilter::FILTER_LINEAR_MIPMAP_LINEAR: return GL_LINEAR_MIPMAP_LINEAR;
	case SamplerFilter::FILTER_LINEAR_MIPMAP_NEAREST: return GL_LINEAR_MIPMAP_NEAREST;
	case SamplerFilter::FILTER_NEAREST: return GL_NEAREST;
	case SamplerFilter::FILTER_NEAREST_MIPMAP_LINEAR: return GL_NEAREST_MIPMAP_LINEAR;
	case SamplerFilter::FILTER_NEAREST_MIPMAP_NEAREST: return GL_NEAREST_MIPMAP_NEAREST;
	default: ERROR_AND_DIE( "Unsupported SamplerFilter in GetOpenGLSamplerFilter!" );
	}
}


//--------------------------------------------------------------------------------------------------------------
unsigned int GetOpenGLSamplerWrapMode( SamplerWrapMode engineSampleFilter )
{
	switch ( engineSampleFilter )
	{
	case SamplerWrapMode::WRAP_CLAMP_TO_BORDER: return GL_CLAMP_TO_BORDER;
	case SamplerWrapMode::WRAP_CLAMP_TO_EDGE: return GL_CLAMP_TO_EDGE;
	case SamplerWrapMode::WRAP_MIRRORED_REPEAT: return GL_MIRRORED_REPEAT;
	case SamplerWrapMode::WRAP_REPEAT: return GL_REPEAT;
	default: ERROR_AND_DIE( "Unsupported SamplerWrapMode in GetOpenGLSamplerWrapMode!" );
	}
}


//--------------------------------------------------------------------------------------------------------------
unsigned int Sampler::RegisterSampler( SamplerFilter minFilter, SamplerFilter magFilter, SamplerWrapMode uWrap, SamplerWrapMode vWrap ) const
{
	//minFilter: when it's too small to grab, i.e. 1 fragment exceeds 1 texel.
	//magFilter: when it's too big, how it stretches/magnifies.
	//uWrap: if u < 0 or u > 1, how it behaves.
	//vWrap: same on v.

	GLuint id;
	glGenSamplers( 1, &id );

	glSamplerParameteri( id, GL_TEXTURE_MIN_FILTER, GetOpenGLSamplerFilter( minFilter ) );
	glSamplerParameteri( id, GL_TEXTURE_MAG_FILTER, GetOpenGLSamplerFilter( magFilter ) );
	glSamplerParameteri( id, GL_TEXTURE_WRAP_S, GetOpenGLSamplerWrapMode( uWrap ) );
	glSamplerParameteri( id, GL_TEXTURE_WRAP_T, GetOpenGLSamplerWrapMode( vWrap ) );

	return id;
}
