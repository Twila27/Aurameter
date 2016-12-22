#pragma once
#include <vector>
#include "Engine/EngineCommon.hpp"

//-----------------------------------------------------------------------------
class Force;
class EphanovParticle;


//-----------------------------------------------------------------------------
class LinearDynamicsState //Future variant or subclass of an entity's rigid-body PhysicsComponent. Distinct from AngularDynamics (see MP3 notes).
{
public:

	LinearDynamicsState( const Vector3f& position = Vector3f::ZERO, const Vector3f& velocity = Vector3f::ZERO )
		: m_position( position )
		, m_velocity( velocity )
	{
	}
	~LinearDynamicsState();

	void StepWithForwardEuler( float mass, float deltaSeconds );
	void StepWithVerlet( float mass, float deltaSeconds );
	Vector3f GetPosition() const { return m_position; }
	Vector3f GetVelocity() const { return m_velocity; }
	void SetPosition( const Vector3f& newPos ) { m_position = newPos; }
	void SetVelocity( const Vector3f& newVel ) { m_velocity = newVel; }
	void AddForce( Force* newForce ) { m_forces.push_back( newForce ); }
	void GetForces( std::vector< Force* >& out_forces ) { out_forces = m_forces; }
	void ClearForces( bool keepGravity = true );

private:

	Vector3f m_position;
	Vector3f m_velocity;
	std::vector<Force*> m_forces; //i.e. All forces acting on whatever this LDS is attached to.

	LinearDynamicsState dStateForMass( float mass ) const; //Solves accel, for use in Step() integrators.
	Vector3f CalcNetForceForMass( float mass ) const; //Used by dState(), gets Sigma[F] by looping m_forces.
};
