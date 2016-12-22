#pragma once
#include "Engine/EngineCommon.hpp"

enum CullMode
{
	CULL_MODE_NONE,
	CULL_MODE_FRONT,
	CULL_MODE_BACK,
	NUM_CULL_MODES
};


enum BlendMode
{
	BLEND_MODE_ZERO = 0, 
	BLEND_MODE_ONE,
	BLEND_MODE_SOURCE_COLOR,
	BLEND_MODE_ONE_MINUS_SOURCE_COLOR,
	BLEND_MODE_SOURCE_ALPHA,
	BLEND_MODE_ONE_MINUS_SOURCE_ALPHA,
	BLEND_MODE_DEST_ALPHA,
	BLEND_MODE_ONE_MINUS_DEST_ALPHA,
	NUM_BLEND_MODES
};


enum DepthTestCompareMode
{
	DEPTH_COMPARE_MODE_NEVER,
	DEPTH_COMPARE_MODE_LESS,
	DEPTH_COMPARE_MODE_LEQUAL,
	DEPTH_COMPARE_MODE_GREATER,
	DEPTH_COMPARE_MODE_GEQUAL,
	DEPTH_COMPARE_MODE_EQUAL,
	DEPTH_COMPARE_MODE_NOTEQUAL,
	DEPTH_COMPARE_MODE_ALWAYS,
	NUM_DEPTH_COMPARE_MODES
};


enum FillMode
{
	FILL_MODE_POINT,
	FILL_MODE_LINE,
	FILL_MODE_FULL,
	NUM_FILL_MODES
};


struct RenderStateOptions
{
	RenderStateOptions( CullMode mode, BlendMode srcMode, BlendMode destMode, DepthTestCompareMode zTest, bool zWriteDepths )
		: m_cullMode( mode )
		, m_sourceBlendMode( srcMode )
		, m_destinationBlendMode( destMode )
		, m_depthTest( zTest )
		, m_writeDepths( zWriteDepths )
	{
	}

	BlendMode m_sourceBlendMode;
	BlendMode m_destinationBlendMode;
	CullMode m_cullMode;
	DepthTestCompareMode m_depthTest;
	bool m_writeDepths;

	void DisableAll() const; //e.g. to unbind from last state binding call.
	void DisableBlending() const;
	void DisableCulling() const;
	void DisableDepthTesting() const;

	void EnableCulling() const;
	void EnableDepthTesting() const;
	void SetBlendMode( BlendMode src, BlendMode dest ) const;
	void SetCullMode( CullMode mode ) const;
	void SetDepthTest( DepthTestCompareMode zTest, bool writeDepths = true ) const;
};


class RenderState //Encapsulates remembering all the glEnable flags and modes for a material.
{
public:
	RenderState( const RenderStateOptions* options ) : m_options( *options ) {}
	RenderState( CullMode culling = CULL_MODE_NONE, BlendMode src = BLEND_MODE_ONE, BlendMode dest = BLEND_MODE_ZERO, 
				 DepthTestCompareMode zTest = DEPTH_COMPARE_MODE_LESS, bool writeDepths = true )
		: m_options( culling, src, dest, zTest, writeDepths )
		, m_isBlendEnabled( true )
		, m_isCullingEnabled( true )
		, m_isDepthTestEnabled( true )
//		, m_isDirty( true ) 
	{
		TODO( "Implement RenderState::m_isDirty flag to save on # gl set calls which stall pipeline!" );
	}

	inline const RenderStateOptions* GetOptions() const { return &m_options; }
	void SetRenderOptions( RenderStateOptions* newState ) { m_options = *newState; /*m_isDirty = true;*/ }
	void DisableBlending() const { m_options.DisableBlending(); }
	void DisableCulling() const { m_options.DisableCulling(); }
	void EnableCulling() const { m_options.EnableCulling(); }
	void SetCulling( CullMode mode ) const { m_options.SetCullMode( mode ); /*m_isDirty = true;*/ }
	void SetDepthTest( DepthTestCompareMode zTest, bool writeDepths = true ) const { m_options.SetDepthTest( zTest, writeDepths ); /*m_isDirty = true;*/ }
	void DisableDepthTesting() const { m_options.DisableDepthTesting(); }
	void EnableDepthTesting() const { m_options.EnableDepthTesting(); }
	void SetBlendMode( BlendMode srcMode, BlendMode destMode ) const { m_options.SetBlendMode( srcMode, destMode ); /*m_isDirty = true;*/ }
	void SetBlendModeAndSave( BlendMode srcMode, BlendMode destMode );
	void SetBlendModeSrc( BlendMode mode ) const { m_options.SetBlendMode( mode, m_options.m_destinationBlendMode ); /*m_isDirty = true;*/ }
	void SetBlendModeDest( BlendMode mode ) const { m_options.SetBlendMode( m_options.m_sourceBlendMode, mode ); /*m_isDirty = true;*/ }

	void Bind() const;

	static const RenderState DEFAULT;

private: //By using bool flags, optimize by not stalling the graphics pipeline for every state bind call.
	RenderStateOptions m_options;
	bool m_isBlendEnabled;
	bool m_isCullingEnabled;
	bool m_isDepthTestEnabled;
//	bool m_isDirty;
};