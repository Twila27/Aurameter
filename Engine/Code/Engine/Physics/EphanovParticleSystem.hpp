#pragma once


#include "Engine/EngineCommon.hpp"
#include "Engine/Physics/EphanovParticle.hpp"


//-----------------------------------------------------------------------------
class Force;


//-----------------------------------------------------------------------------
class EphanovParticleSystem
{
public:

	EphanovParticleSystem( const Vector3f& emitterPosition, EphanovParticleType EphanovParticleType, float EphanovParticleRadius, float EphanovParticleMass,
					float muzzleSpeed, float maxDegreesDownFromWorldUp, float minDegreesDownFromWorldUp, float maxDegreesLeftFromWorldNorth, float minDegreesLeftFromWorldNorth,
					float secondsBetweenEmits, float secondsBeforeEphanovParticlesExpire, unsigned int maxEphanovParticlesEmitted, unsigned int EphanovParticlesEmittedAtOnce );
	~EphanovParticleSystem();

	void RenderThenExpireEphanovParticles();
	void UpdateEphanovParticles( float deltaSeconds );
	void AddForce( Force* newForce ) {
		m_EphanovParticleToEmit.AddForce( newForce );
	}
	float GetSecondsUntilNextEmit() const {
		return m_secondsBetweenEmits - m_secondsPassedSinceLastEmit;
	}

private:

	void StepAndAgeEphanovParticles( float deltaSeconds );
	void EmitEphanovParticles( float deltaSeconds ); //silently emits nothing if not yet time to emit.

	float m_maxDegreesDownFromWorldUp; //"theta" in most spherical-to-Cartesian conversions.
	float m_minDegreesDownFromWorldUp;
	float m_maxDegreesLeftFromWorldNorth; //"phi".
	float m_minDegreesLeftFromWorldNorth;

	float m_muzzleSpeed; //How fast EphanovParticles shoot out.
	float m_secondsPassedSinceLastEmit;
	float m_secondsBetweenEmits;
	float m_secondsBeforeEphanovParticlesExpire;
	unsigned int m_maxEphanovParticlesEmitted;
	unsigned int m_EphanovParticlesEmittedAtOnce; //Destroys oldest one(s) on next emit until emitter can emit this amount. 
										   //No angular velocity right now.
										   //No ability to ignore parent velocity right now.

	Vector3f m_emitterPosition;

	EphanovParticle m_EphanovParticleToEmit;
	std::vector< EphanovParticle* > m_unexpiredEphanovParticles;

	static const Vector3f MAX_EphanovParticle_OFFSET_FROM_EMITTER;
	static SoundID s_emitSoundID;
};
