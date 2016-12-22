#pragma once


#include "Engine/Renderer/Rgba.hpp"


//-----------------------------------------------------------------------------
class LinearDynamicsState;
class Sprite;


//-----------------------------------------------------------------------------
class Particle
{
public:
	Particle()
		: m_physicsInfo( nullptr )
		, m_currentAgeSeconds( 0.f )
	{
	}

	Sprite* m_sprite; //May be an animation.
	LinearDynamicsState* m_physicsInfo;
	float m_mass; //Acceleration calculated from this.
	float m_currentAgeSeconds;
	float m_maxAgeSeconds;
	Vector2f m_scale;
	Rgba m_tint; //Includes alpha.
	//What does size do?
	//Constraints: for now, just spawn at emitter position's center.
};
