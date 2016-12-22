#pragma once


#include <vector>
#include "Engine/EngineCommon.hpp"


//-----------------------------------------------------------------------------
enum EphanovParticleType 
{ 
	/*EphanovParticle_AABB2,*/ 
	EphanovParticle_AABB3, 
	EphanovParticle_SPHERE,
	NUM_EphanovParticle_TYPES
}; //FUTURE IDEAS TODO: Add whatever TheRenderer supports as a Draw call!


//-----------------------------------------------------------------------------
class LinearDynamicsState;
class Force;


//-----------------------------------------------------------------------------
class EphanovParticle //NOTE: NOT an Entity3D because some entities won't need expiration logic.
{
public:

	EphanovParticle( EphanovParticleType renderType, float mass, float secondsToLive, float renderRadius )
		: m_mass( mass )
		, m_secondsToLive( secondsToLive )
		, m_renderType( renderType )
		, m_renderRadius( renderRadius )
		, m_state( nullptr )
		, m_isPinned( false )
	{
	}
	~EphanovParticle();

	void GetEphanovParticleState( LinearDynamicsState& out_state ) const;
	void SetEphanovParticleState( LinearDynamicsState* newState ) { m_state = newState; }
	void Render();
	void StepAndAge( float deltaSeconds );
	void SetIsExpired( bool newVal ) { m_secondsToLive = newVal ? -1.f : 1.f; }
	bool IsExpired() const { return m_secondsToLive <= 0.f; }
	void GetForces( std::vector< Force* >& out_forces ) const;
	void ResetForces( bool keepGravity = true );
	void AddForce( Force* newForce );
	void CloneForcesFromEphanovParticle( const EphanovParticle* sourceEphanovParticle );

	//The return bool value indicates failure <=> EphanovParticle has no m_state.
	bool GetPosition( Vector3f& out_position );
	bool SetPosition( const Vector3f& newPosition );
	bool Translate( const Vector3f& newPosition );
	bool GetVelocity( Vector3f& out_velocity );
	bool SetVelocity( const Vector3f& newVelocity );

	bool GetIsPinned() const { return m_isPinned; }
	void SetIsPinned( bool newVal ) { m_isPinned = newVal; }
	void ToggleIsPinned() { m_isPinned = !m_isPinned; }

	LinearDynamicsState* m_state;


private:
	bool m_isPinned;

	float m_mass;
	float m_secondsToLive;

	EphanovParticleType m_renderType;
	float m_renderRadius;
};


