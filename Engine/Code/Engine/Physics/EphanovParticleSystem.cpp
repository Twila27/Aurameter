#include "Engine/Physics/EphanovParticleSystem.hpp"
#include "Engine/Audio/TheAudio.hpp"
#include "Engine/Renderer/TheRenderer.hpp"
#include "Engine/Physics/PhysicsUtils.hpp"
#include "Engine/Math/MathUtils.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC const Vector3f EphanovParticleSystem::MAX_EphanovParticle_OFFSET_FROM_EMITTER = Vector3f::ZERO;
STATIC SoundID EphanovParticleSystem::s_emitSoundID = 0;


//--------------------------------------------------------------------------------------------------------------
EphanovParticleSystem::EphanovParticleSystem( const Vector3f& emitterPosition, EphanovParticleType EphanovParticleType, float EphanovParticleRadius, float EphanovParticleMass,
				float muzzleSpeed, float maxDegreesDownFromWorldUp, float minDegreesDownFromWorldUp, float maxDegreesLeftFromWorldNorth, float minDegreesLeftFromWorldNorth,
				float secondsBetweenEmits, float secondsBeforeEphanovParticlesExpire, unsigned int maxEphanovParticlesEmitted, unsigned int EphanovParticlesEmittedAtOnce )
	: m_emitterPosition( emitterPosition )
	, m_muzzleSpeed( muzzleSpeed )
	, m_maxDegreesDownFromWorldUp( maxDegreesDownFromWorldUp )
	, m_minDegreesDownFromWorldUp( minDegreesDownFromWorldUp )
	, m_maxDegreesLeftFromWorldNorth( maxDegreesLeftFromWorldNorth )
	, m_minDegreesLeftFromWorldNorth( minDegreesLeftFromWorldNorth )
	, m_EphanovParticleToEmit( EphanovParticleType, EphanovParticleMass, secondsBeforeEphanovParticlesExpire, EphanovParticleRadius )
	, m_secondsBetweenEmits( secondsBetweenEmits )
	, m_secondsBeforeEphanovParticlesExpire( secondsBeforeEphanovParticlesExpire )
	, m_maxEphanovParticlesEmitted( maxEphanovParticlesEmitted )
	, m_EphanovParticlesEmittedAtOnce( EphanovParticlesEmittedAtOnce )
	, m_secondsPassedSinceLastEmit( 0.f )
{
	GUARANTEE_OR_DIE( m_EphanovParticlesEmittedAtOnce <= m_maxEphanovParticlesEmitted, "Error in EphanovParticleSystem ctor, amount to emit at once exceeds max amount to emit." ); //Else infinite loop in EmitEphanovParticles().
	m_EphanovParticleToEmit.SetEphanovParticleState( new LinearDynamicsState( emitterPosition, Vector3f::ZERO ) ); //So we can add forces to it prior to emission if requested.
	EphanovParticleSystem::s_emitSoundID = g_theAudio->CreateOrGetSound( "Data/Audio/Explo_EnergyFireball01.wav" );
}


//--------------------------------------------------------------------------------------------------------------
EphanovParticleSystem::~EphanovParticleSystem()
{
	for ( unsigned int EphanovParticleIndex = 0; EphanovParticleIndex < m_unexpiredEphanovParticles.size(); EphanovParticleIndex++ )
	{
		EphanovParticle* currentEphanovParticle = m_unexpiredEphanovParticles[ EphanovParticleIndex ];
		if ( currentEphanovParticle != nullptr )
		{
			delete currentEphanovParticle;
			currentEphanovParticle = nullptr;
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
void EphanovParticleSystem::RenderThenExpireEphanovParticles()
{
	for ( auto EphanovParticleIter = m_unexpiredEphanovParticles.begin(); EphanovParticleIter != m_unexpiredEphanovParticles.end(); )
	{
		EphanovParticle* currentEphanovParticle = *EphanovParticleIter;
		currentEphanovParticle->Render();
		if ( currentEphanovParticle->IsExpired() )
		{
			delete currentEphanovParticle;
			currentEphanovParticle = nullptr;
			EphanovParticleIter = m_unexpiredEphanovParticles.erase( EphanovParticleIter );
		}
		else ++EphanovParticleIter;
	}
}


//--------------------------------------------------------------------------------------------------------------
void EphanovParticleSystem::UpdateEphanovParticles( float deltaSeconds )
{
	StepAndAgeEphanovParticles( deltaSeconds );
	EmitEphanovParticles( deltaSeconds );
}


//--------------------------------------------------------------------------------------------------------------
void EphanovParticleSystem::StepAndAgeEphanovParticles( float deltaSeconds )
{
	for ( unsigned int EphanovParticleIndex = 0; EphanovParticleIndex < m_unexpiredEphanovParticles.size(); EphanovParticleIndex++ )
	{
		m_unexpiredEphanovParticles[ EphanovParticleIndex ]->StepAndAge( deltaSeconds );
	}
}


//--------------------------------------------------------------------------------------------------------------
void EphanovParticleSystem::EmitEphanovParticles( float deltaSeconds )
{
	if ( m_secondsPassedSinceLastEmit >= m_secondsBetweenEmits )
	{
		m_secondsPassedSinceLastEmit = 0.f;

		//Prep for emit by erasing oldest EphanovParticles to make enough room.
		unsigned int amountUnexpiredEphanovParticles = m_unexpiredEphanovParticles.size();

		while ( amountUnexpiredEphanovParticles + m_EphanovParticlesEmittedAtOnce > m_maxEphanovParticlesEmitted )
		{
			//Remove earliest objects pushed--means removing from the front of a structure walked per-frame with items potentially being removed at its middle due their expiration. Any choice seems slow.
			m_unexpiredEphanovParticles.erase( m_unexpiredEphanovParticles.begin() );
			amountUnexpiredEphanovParticles = m_unexpiredEphanovParticles.size();
		}

		//Actual emit.
		for ( unsigned int iterationNum = 0; iterationNum < m_EphanovParticlesEmittedAtOnce; iterationNum++ )
		{
			EphanovParticle* newEphanovParticle = new EphanovParticle( m_EphanovParticleToEmit );
			Vector3f newEphanovParticlePosition = m_emitterPosition; //Below offset to position allows us not to just have EphanovParticles emitting outward in "bands".
			newEphanovParticlePosition.x += MAX_EphanovParticle_OFFSET_FROM_EMITTER.x * GetRandomFloatInRange( -1.f, 1.f );
			newEphanovParticlePosition.y += MAX_EphanovParticle_OFFSET_FROM_EMITTER.y * GetRandomFloatInRange( -1.f, 1.f );
			newEphanovParticlePosition.z += MAX_EphanovParticle_OFFSET_FROM_EMITTER.z * GetRandomFloatInRange( -1.f, 1.f );

			Vector3f muzzleVelocity; //Below follows spherical-to-Cartesian conversion formulas.
			float spanDegreesDownFromWorldUp = m_maxDegreesDownFromWorldUp - m_minDegreesDownFromWorldUp;
			float spanDegreesLeftFromWorldNorth = m_maxDegreesLeftFromWorldNorth - m_minDegreesLeftFromWorldNorth;
			muzzleVelocity.x = m_muzzleSpeed
				* SinDegrees( ( spanDegreesDownFromWorldUp		* GetRandomFloatZeroTo( 1.0f ) ) + m_minDegreesDownFromWorldUp )
				* CosDegrees( ( spanDegreesLeftFromWorldNorth	* GetRandomFloatZeroTo( 1.0f ) ) + m_minDegreesLeftFromWorldNorth );
			muzzleVelocity.y = m_muzzleSpeed
				* SinDegrees( ( spanDegreesDownFromWorldUp		* GetRandomFloatZeroTo( 1.0f ) ) + m_minDegreesDownFromWorldUp )
				* SinDegrees( ( spanDegreesLeftFromWorldNorth	* GetRandomFloatZeroTo( 1.0f ) ) + m_minDegreesLeftFromWorldNorth );
			muzzleVelocity.z = m_muzzleSpeed
				* CosDegrees( ( spanDegreesDownFromWorldUp		* GetRandomFloatZeroTo( 1.0f ) ) + m_minDegreesDownFromWorldUp ); //Embeds assumption z is world-up? Would it work if using y-up, just rotated by 90deg?

			newEphanovParticle->SetEphanovParticleState( new LinearDynamicsState( newEphanovParticlePosition, muzzleVelocity ) );
			newEphanovParticle->CloneForcesFromEphanovParticle( &m_EphanovParticleToEmit );
			m_unexpiredEphanovParticles.push_back( newEphanovParticle );
		}

		g_theAudio->PlaySound( s_emitSoundID );
	}
	else m_secondsPassedSinceLastEmit += deltaSeconds;
}
