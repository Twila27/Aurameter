#include "Engine/Physics/EphanovParticle.hpp"
#include "Engine/Physics/PhysicsUtils.hpp"
#include "Engine/Physics/Forces.hpp"
#include "Engine/Renderer/TheRenderer.hpp"


//--------------------------------------------------------------------------------------------------------------
EphanovParticle::~EphanovParticle()
{
	if ( m_state != nullptr )
	{
		delete m_state;
		m_state = nullptr;
	}
}


//--------------------------------------------------------------------------------------------------------------
void EphanovParticle::Render()
{
	switch ( m_renderType )
	{
	case EphanovParticle_SPHERE:
		g_theRenderer->DrawAnthonyCloudySphere( m_state->GetPosition(), m_renderRadius, 20.f );
		break;
	case EphanovParticle_AABB3:
		Vector3f EphanovParticlePos = m_state->GetPosition();
		Vector3f offsetToCorners = Vector3f( m_renderRadius );
		Vector3f EphanovParticleMins = EphanovParticlePos - offsetToCorners;
		Vector3f EphanovParticleMaxs = EphanovParticlePos + offsetToCorners;
		g_theRenderer->DrawShadedAABB( VertexGroupingRule::AS_TRIANGLES, AABB3f( EphanovParticleMins, EphanovParticleMaxs ), Rgba::GREEN, Rgba::WHITE, Rgba::BLACK, Rgba::RED );
		break;
		//FUTURE IDEAS TODO: add more render types!
	}
}


//--------------------------------------------------------------------------------------------------------------
void EphanovParticle::StepAndAge( float deltaSeconds )
{
	//Commented out because the expire-after-x-seconds logic is not needed for the cloth simulation.

	//	m_state->StepWithForwardEuler( m_mass, deltaSeconds );
	m_state->StepWithVerlet( m_mass, deltaSeconds );
	//	m_secondsToLive -= deltaSeconds;
}


//--------------------------------------------------------------------------------------------------------------
void EphanovParticle::GetForces( std::vector<Force*>& out_forces ) const
{
	if ( m_state != nullptr )
		m_state->GetForces( out_forces );
}


//--------------------------------------------------------------------------------------------------------------
void EphanovParticle::AddForce( Force* newForce )
{
	if ( m_state != nullptr )
		m_state->AddForce( newForce );
}


//--------------------------------------------------------------------------------------------------------------
void EphanovParticle::CloneForcesFromEphanovParticle( const EphanovParticle* sourceEphanovParticle )
{
	if ( m_state == nullptr ) return;

	std::vector< Force*> sourceEphanovParticleForces;
	sourceEphanovParticle->GetForces( sourceEphanovParticleForces );

	for ( unsigned int forceIndex = 0; forceIndex < sourceEphanovParticleForces.size(); forceIndex++ )
		m_state->AddForce( sourceEphanovParticleForces[ forceIndex ]->GetCopy() );
}

//--------------------------------------------------------------------------------------------------------------
bool EphanovParticle::GetPosition( Vector3f& out_position )
{
	if ( m_state == nullptr )
		return false;

	out_position = m_state->GetPosition();
	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool EphanovParticle::SetPosition( const Vector3f& newPosition )
{
	if ( m_state == nullptr )
		return false;

	m_state->SetPosition( newPosition );
	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool EphanovParticle::Translate( const Vector3f& translation )
{
	if ( m_state == nullptr )
		return false;

	m_state->SetPosition( m_state->GetPosition() + translation );
	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool EphanovParticle::GetVelocity( Vector3f& out_position )
{
	if ( m_state == nullptr )
		return false;

	out_position = m_state->GetVelocity();
	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool EphanovParticle::SetVelocity( const Vector3f& newVelocity )
{
	if ( m_state == nullptr )
		return false;

	m_state->SetVelocity( newVelocity );
	return true;
}


//--------------------------------------------------------------------------------------------------------------
void EphanovParticle::GetEphanovParticleState( LinearDynamicsState& out_state ) const
{
	out_state = *m_state;
}


//--------------------------------------------------------------------------------------------------------------
void EphanovParticle::ResetForces( bool keepGravity /*= true */ )
{
	m_state->ClearForces( keepGravity );
}
