#include "Engine/Renderer/RenderState.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>


//--------------------------------------------------------------------------------------------------------------
const RenderState RenderState::DEFAULT = RenderState();


//--------------------------------------------------------------------------------------------------------------
unsigned int GetOpenGLCullMode( CullMode engineCullMode )
{
	switch ( engineCullMode )
	{
	case CullMode::CULL_MODE_BACK: return GL_BACK;
	case CullMode::CULL_MODE_FRONT: return GL_FRONT;
	default: ERROR_AND_DIE( "Unsupported CullMode in GetOpenGLCullMode!" );
	}
}


//--------------------------------------------------------------------------------------------------------------
unsigned int GetOpenGLBlendMode( BlendMode engineBlendMode )
{
	switch ( engineBlendMode )
	{
	case BlendMode::BLEND_MODE_ZERO: return GL_ZERO;
	case BlendMode::BLEND_MODE_ONE: return GL_ONE;
	case BlendMode::BLEND_MODE_SOURCE_COLOR: return GL_SRC_COLOR;
	case BlendMode::BLEND_MODE_ONE_MINUS_SOURCE_COLOR: return GL_ONE_MINUS_SRC_COLOR;
	case BlendMode::BLEND_MODE_SOURCE_ALPHA: return GL_SRC_ALPHA;
	case BlendMode::BLEND_MODE_ONE_MINUS_SOURCE_ALPHA: return GL_ONE_MINUS_SRC_ALPHA;
	case BlendMode::BLEND_MODE_DEST_ALPHA: return GL_DST_ALPHA;
	case BlendMode::BLEND_MODE_ONE_MINUS_DEST_ALPHA: return GL_ONE_MINUS_DST_ALPHA;
	default: ERROR_AND_DIE( "Unsupported BlendMode in GetOpenGLBlendMode!" );
	}
}


//--------------------------------------------------------------------------------------------------------------
unsigned int GetOpenGLDepthTestCompareMode( DepthTestCompareMode engineDepthTestCompareMode )
{
	switch ( engineDepthTestCompareMode )
	{
	case DepthTestCompareMode::DEPTH_COMPARE_MODE_NEVER: return GL_NEVER;
	case DepthTestCompareMode::DEPTH_COMPARE_MODE_LESS: return GL_LESS;
	case DepthTestCompareMode::DEPTH_COMPARE_MODE_LEQUAL: return GL_LEQUAL;
	case DepthTestCompareMode::DEPTH_COMPARE_MODE_GREATER: return GL_GREATER;
	case DepthTestCompareMode::DEPTH_COMPARE_MODE_GEQUAL: return GL_GEQUAL;
	case DepthTestCompareMode::DEPTH_COMPARE_MODE_EQUAL: return GL_EQUAL;
	case DepthTestCompareMode::DEPTH_COMPARE_MODE_NOTEQUAL: return GL_NOTEQUAL;
	case DepthTestCompareMode::DEPTH_COMPARE_MODE_ALWAYS: return GL_ALWAYS;
	default: ERROR_AND_DIE( "Unsupported DepthTestCompareMode in GetOpenGLDepthTestCompareMode!" );
	}
}


//--------------------------------------------------------------------------------------------------------------
unsigned int GetOpenGLFillMode( FillMode engineFillMode )
{
	switch ( engineFillMode )
	{
	case FillMode::FILL_MODE_POINT: return GL_POINT;
	case FillMode::FILL_MODE_LINE: return GL_LINE;
	case FillMode::FILL_MODE_FULL: return GL_FILL;
	default: ERROR_AND_DIE( "Unsupported FillMode in GetOpenGLFillMode!" );
	}
}


//--------------------------------------------------------------------------------------------------------------
void RenderStateOptions::DisableAll() const
{
	glDisable( GL_BLEND );
	glDisable( GL_CULL_FACE );
	glDisable( GL_FILL );
	glDisable( GL_DEPTH_TEST );
}


//--------------------------------------------------------------------------------------------------------------
void RenderStateOptions::DisableBlending() const
{
	glDisable( GL_BLEND );
}


//--------------------------------------------------------------------------------------------------------------
void RenderStateOptions::DisableCulling() const
{
	glDisable( GL_CULL_FACE );
}


//--------------------------------------------------------------------------------------------------------------
void RenderStateOptions::DisableDepthTesting() const
{
	glDisable( GL_DEPTH_TEST );
}


//--------------------------------------------------------------------------------------------------------------
void RenderStateOptions::EnableDepthTesting() const
{
	glEnable( GL_DEPTH_TEST );
}


//--------------------------------------------------------------------------------------------------------------
void RenderStateOptions::EnableCulling() const
{
	glEnable( GL_CULL_FACE );
}


//--------------------------------------------------------------------------------------------------------------
void RenderStateOptions::SetBlendMode( BlendMode src, BlendMode dest ) const
{
	glEnable( GL_BLEND );

	glBlendFunc( GetOpenGLBlendMode( src ), GetOpenGLBlendMode( dest ) );
}


//--------------------------------------------------------------------------------------------------------------
void RenderState::SetBlendModeAndSave( BlendMode src, BlendMode dest )
{
	glEnable( GL_BLEND );

	glBlendFunc( GetOpenGLBlendMode( src ), GetOpenGLBlendMode( dest ) );

	m_options.m_sourceBlendMode = src;
	m_options.m_destinationBlendMode = dest;
}


//--------------------------------------------------------------------------------------------------------------
void RenderStateOptions::SetCullMode( CullMode mode ) const
{
	if ( mode == CULL_MODE_NONE )
	{
		DisableCulling();
		return;
	}

	glEnable( GL_CULL_FACE );

	glCullFace( GetOpenGLCullMode( mode ) );
}


//--------------------------------------------------------------------------------------------------------------
void RenderStateOptions::SetDepthTest( DepthTestCompareMode zTest, bool writeDepths ) const
{
	glEnable( GL_DEPTH_TEST );

	glDepthFunc( GetOpenGLDepthTestCompareMode( zTest ) );

	glDepthMask( writeDepths );
}


//--------------------------------------------------------------------------------------------------------------
void RenderState::Bind() const
{
// 	if ( m_isDirty )
// 	{
		if ( m_isBlendEnabled )
			m_options.SetBlendMode( m_options.m_sourceBlendMode, m_options.m_destinationBlendMode );
		else DisableBlending();

		if ( m_isCullingEnabled )
			m_options.SetCullMode( m_options.m_cullMode );
		else DisableCulling();

		if ( m_isDepthTestEnabled )
			m_options.SetDepthTest( m_options.m_depthTest, m_options.m_writeDepths );
		else DisableDepthTesting();

// 		m_isDirty = false;
// 	}
}
