#pragma once


#include "Engine/Renderer/Rgba.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Core/Command.hpp"
#include "Engine/Memory/UntrackedAllocator.hpp"
#include <list>


//-----------------------------------------------------------------------------
struct DebugRenderCommand;
//extern std::list< DebugRenderCommand*, UntrackedAllocator<DebugRenderCommand*> >* g_theDebugRenderCommands; //Because these are frequently deleted at random upon expiration.
extern std::list< DebugRenderCommand* >* g_theDebugRenderCommands; //Because these are frequently deleted at random upon expiration.


//-----------------------------------------------------------------------------
void RenderThenExpireDebugCommands3D();
void UpdateDebugCommands( float deltaSeconds );
void ClearDebugCommands();
void AddDebugRenderCommand( DebugRenderCommand* newCommand );


//-----------------------------------------------------------------------------
void DebugRenderClearCommands( Command& /*args*/ );


//-----------------------------------------------------------------------------
enum DepthMode { 
	DEPTH_TEST_ON,
	DEPTH_TEST_OFF,
	DEPTH_TEST_DUAL, //Drawn once on (brighter/bigger), once off (fainter/smaller).
	NUM_DEPTH_MODES
};


//-----------------------------------------------------------------------------
struct DebugRenderCommand
{
	Rgba m_color;
	float m_secondsToLive;
	DepthMode m_depthMode;
	float m_lineThickness;

	DebugRenderCommand( float secondsToLive = 0.f, 
				   DepthMode depthMode = DEPTH_TEST_ON, 
				   const Rgba& color = Rgba::WHITE,
				   float lineThickness = 1.f )
		: m_color( color )
		, m_secondsToLive( secondsToLive )
		, m_depthMode( depthMode ) 
		, m_lineThickness( lineThickness )
	{
	}

	bool IsExpired() { return m_secondsToLive <= 0.f; }
	void Update( float deltaSeconds ) { m_secondsToLive -= deltaSeconds; }
	void virtual Render() = 0;
};

//-----------------------------------------------------------------------------
struct DebugRenderCommandPoint : public DebugRenderCommand
{
	Vector3f m_position;
	float m_sizeScalar;

	DebugRenderCommandPoint( const Vector3f& position, 
						float secondsToLive, DepthMode depthMode, const Rgba& color, float lineThickness, float sizeScalar = .1f )
		: DebugRenderCommand( secondsToLive, depthMode, color, lineThickness )
		, m_position( position )
		, m_sizeScalar( sizeScalar )
	{
	}

	void Render() override;
};


//-----------------------------------------------------------------------------
struct DebugRenderCommandLine : public DebugRenderCommand
{
	Vector3f m_startPos;
	Vector3f m_endPos;

	DebugRenderCommandLine( const Vector3f& startPos,
					   const Vector3f& endPos,
					   float secondsToLive, DepthMode depthMode, const Rgba& color, float lineThickness )
		: DebugRenderCommand( secondsToLive, depthMode, color, lineThickness )
		, m_startPos( startPos )
		, m_endPos( endPos ) 
	{
	}

	void Render() override;
};


//-----------------------------------------------------------------------------
struct DebugRenderCommandArrow : public DebugRenderCommand
{
	Vector3f m_startPos;
	Vector3f m_endPos;

	DebugRenderCommandArrow( const Vector3f& startPos,
						const Vector3f& endPos,
						float secondsToLive, DepthMode depthMode, const Rgba& color, float lineThickness )
		: DebugRenderCommand( secondsToLive, depthMode, color, lineThickness )
		, m_startPos( startPos )
		, m_endPos( endPos ) 
	{
	}

	void Render() override;
};


//-----------------------------------------------------------------------------
struct DebugRenderCommandAABB3 : public DebugRenderCommand
{
	AABB3f m_bounds;
	Rgba m_edgeColor;
	bool m_drawFilled;

	DebugRenderCommandAABB3( const AABB3f& bounds,
						bool drawFilled,
						float secondsToLive, DepthMode depthMode, float lineThickness, const Rgba& edgeColor, const Rgba& fillColor )
		: DebugRenderCommand( secondsToLive, depthMode, fillColor, lineThickness )
		, m_bounds( bounds )
		, m_edgeColor( edgeColor )
		, m_drawFilled( drawFilled )
	{
	}

	void Render() override;
};


//-----------------------------------------------------------------------------
struct DebugRenderCommandSphere : public DebugRenderCommand
{
	Vector3f m_centerPos;
	float m_radius;

	DebugRenderCommandSphere( const Vector3f& centerPos,
						 float radius,
						 float secondsToLive, DepthMode depthMode, const Rgba& color, float lineThickness )
		: DebugRenderCommand( secondsToLive, depthMode, color, lineThickness )
		, m_centerPos( centerPos )
		, m_radius( radius )
	{
	}

	void Render() override;
};


//-----------------------------------------------------------------------------
struct DebugRenderCommandBasis : public DebugRenderCommand
{
	Vector3f m_originPos;
	float m_axisLength;
	bool m_showZ;

	DebugRenderCommandBasis( const Vector3f& originPos,
						float axisLength,
						bool showZ,
						float secondsToLive, DepthMode depthMode, float lineThickness )
		: DebugRenderCommand( secondsToLive, depthMode, Rgba::WHITE, lineThickness )
		, m_originPos( originPos )
		, m_axisLength( axisLength )
		, m_showZ( showZ )
	{
	}

	void Render() override;
};