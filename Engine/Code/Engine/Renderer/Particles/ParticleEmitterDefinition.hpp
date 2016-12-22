#pragma once


#include "Engine/Renderer/Rgba.hpp"
#include "Engine/Math/Interval.hpp"
#include "Engine/Renderer/Particles/Particle.hpp"
#include "Engine/Renderer/RenderState.hpp"


//-----------------------------------------------------------------------------
class Sprite;
class SpriteResource;
class AnimatedSpriteSequence;


//-----------------------------------------------------------------------------
enum ParticleBlendState
{
	PARTICLE_BLEND_STATE_ALPHA,
	PARTICLE_BLEND_STATE_ALPHA_ADDITIVE,
	NUM_PARTICLE_BLEND_STATES
};


//-----------------------------------------------------------------------------
class ParticleEmitterDefinition
{
public:
	static ParticleEmitterDefinition* Create( ResourceID spriteResourceID, bool isAnimated );


public:
	int m_initialSpawnCount;
	float m_secondsPerSpawn;
	Interval<float> m_lifetimeSeconds;
	Rgba m_tint;
	Interval<Vector2f> m_initialVelocity;
	Interval<float> m_mass; //Affects acceleration.
	Interval<Vector2f> m_initialScale;

	std::string GetName() const { return m_name; }
	Sprite* GetSprite( const WorldCoords2D& position, const Rgba& tint ) const;
	RenderState* GetRenderState() { return &m_renderState; }
	bool IsLooping() const { return m_secondsPerSpawn > 0.f; }

	void SetBlendState( ParticleBlendState );
	void Update( std::vector<Particle>& particles, float deltaSeconds ) const;
	void Destroy( std::vector<Particle>& particles ) const;
	int Spawn( std::vector<Particle>& particles, const WorldCoords2D& position, float& secondsSinceLastSpawn ) const;
	Particle SpawnParticle( const WorldCoords2D& position ) const;


private:
	ParticleEmitterDefinition() 
		: m_initialSpawnCount( 0 )
		, m_secondsPerSpawn( 0.f ) //Default support for non-looping.
		, m_initialVelocity( Vector2f::ZERO )
		, m_mass( 1.f )
		, m_renderState( CULL_MODE_BACK, BLEND_MODE_SOURCE_ALPHA, BLEND_MODE_ONE_MINUS_SOURCE_ALPHA, DEPTH_COMPARE_MODE_LESS, false )
	{
	}
	ResourceID m_name;
	RenderState m_renderState; //Alpha, additive, etc. blend modes.

	bool m_isAnimated;
	SpriteResource const* m_spriteResource; //One sprite to one emitter limit.
	AnimatedSpriteSequence const* m_animationResource; //Used instead of m_sprite if isAnimated.
};
