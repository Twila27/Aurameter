#include "Engine/Renderer/Particles/ParticleEmitterDefinition.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Renderer/AnimatedSprite.hpp"
#include "Engine/Physics/PhysicsUtils.hpp"
#include "Engine/Renderer/ResourceDatabase.hpp"



//--------------------------------------------------------------------------------------------------------------
STATIC ParticleEmitterDefinition* ParticleEmitterDefinition::Create( ResourceID spriteResourceID, bool isAnimated )
{
	static int numInvocation = 0;

	ParticleEmitterDefinition* ped = new ParticleEmitterDefinition();
	ped->m_isAnimated = isAnimated;
	if ( isAnimated )
		ped->m_animationResource = ResourceDatabase::Instance()->GetSpriteSequence( spriteResourceID );
	else
		ped->m_spriteResource = ResourceDatabase::Instance()->GetSpriteResource( spriteResourceID );

	ped->m_name = spriteResourceID + Stringf( "%d_Emitter", numInvocation );

	return ped;
}


//--------------------------------------------------------------------------------------------------------------
Sprite* ParticleEmitterDefinition::GetSprite( const WorldCoords2D& position, const Rgba& tint ) const
{
	if ( m_isAnimated )
		return AnimatedSprite::Create( m_animationResource->GetID(), position, tint );
	else
		return Sprite::Create( m_spriteResource->GetID(), position, tint );
}


//--------------------------------------------------------------------------------------------------------------
void ParticleEmitterDefinition::SetBlendState( ParticleBlendState pbs )
{
	switch ( pbs )
	{
		case PARTICLE_BLEND_STATE_ALPHA:
			m_renderState.SetBlendModeAndSave( BLEND_MODE_SOURCE_ALPHA, BLEND_MODE_ONE_MINUS_SOURCE_ALPHA );
			break;
		case PARTICLE_BLEND_STATE_ALPHA_ADDITIVE:
			m_renderState.SetBlendModeAndSave( BLEND_MODE_SOURCE_ALPHA, BLEND_MODE_ONE ); //NOTE: Use ONE, ONE if non-alpha sprites are used!
			break;
	}
}


//--------------------------------------------------------------------------------------------------------------
void ParticleEmitterDefinition::Update( std::vector<Particle>& particles, float deltaSeconds ) const
{
	for ( Particle& p : particles )
	{
		p.m_currentAgeSeconds += deltaSeconds;

		p.m_physicsInfo->StepWithVerlet( 1.f, deltaSeconds );
		
		p.m_tint.alphaOpacity = static_cast<byte_t>( 255.f * ( p.m_currentAgeSeconds / p.m_maxAgeSeconds ) );
	}
}


//--------------------------------------------------------------------------------------------------------------
void ParticleEmitterDefinition::Destroy( std::vector<Particle>& particles ) const
{
	for ( size_t i = 0; i < particles.size(); ++i )
	{
		if ( particles[ i ].m_currentAgeSeconds > particles[ i ].m_maxAgeSeconds )
		{
			particles[ i ].m_sprite->Disable();
			particles[ i ] = particles[ particles.size() - 1 ]; //Takes the thing from the end and moves it to the front. This way we need not shift everything down.
				//Helps with cache lines because you're accessing the tail and the head moving towards each other, so the cache lines are likely closer and will eventually be the same line.

			--i;

			particles.pop_back(); //Vectors shouldn't ever shrink from this, so we're saving the memory this way.
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
int ParticleEmitterDefinition::Spawn( std::vector<Particle>& particles, const WorldCoords2D& position, float& secondsSinceLastSpawn ) const
{
	int numSpawned = 0;

	while ( ( secondsSinceLastSpawn >= m_secondsPerSpawn ) && ( numSpawned < m_initialSpawnCount ) )
	{
		particles.push_back( ParticleEmitterDefinition::SpawnParticle( position ) );

		secondsSinceLastSpawn -= m_secondsPerSpawn;

		++numSpawned;
	}

	return numSpawned;
}


//--------------------------------------------------------------------------------------------------------------
Particle ParticleEmitterDefinition::SpawnParticle( const WorldCoords2D& position ) const
{
	Particle p;
	p.m_currentAgeSeconds = 0.f;
	p.m_maxAgeSeconds = m_lifetimeSeconds.GetRandomElement();
	p.m_scale = m_initialScale.GetRandomElement();

	LinearDynamicsState* lds = new LinearDynamicsState();
	lds->SetPosition( Vector3f( position.x, position.y, 0.f ) );
	Vector2f vel = m_initialVelocity.GetRandomElement();
	lds->SetVelocity( Vector3f( vel.x, vel.y, 0.f ) );
	p.m_mass = m_mass.GetRandomElement();
	p.m_physicsInfo = lds;

	p.m_tint = m_tint;
	p.m_sprite = GetSprite( position, m_tint );
	p.m_sprite->Enable();
	TODO( "Add special movement logic here via polymorphism." );

	return p;
}
