#include "Engine/Physics/Forces.hpp"
#include "Engine/Physics/PhysicsUtils.hpp"
#include "Engine/Math/MathUtils.hpp"


//--------------------------------------------------------------------------------------------------------------
Vector3f GravityForce::CalcForceForStateAndMass( const LinearDynamicsState * lds, float mass ) const
{
	return CalcDirectionForState( lds ) * CalcMagnitudeForState( lds ) * mass;
}


//--------------------------------------------------------------------------------------------------------------
Vector3f ConstantWindForce::CalcForceForStateAndMass( const LinearDynamicsState * lds, float /*mass*/ ) const
{
	Vector3f windVector = CalcDirectionForState( lds ) * CalcMagnitudeForState( lds );
	Vector3f undampedWindForce = lds->GetVelocity() - windVector;

	return undampedWindForce * -m_dampedness;
}


//--------------------------------------------------------------------------------------------------------------
float WormholeForce::CalcMagnitudeForState( const LinearDynamicsState * lds ) const
{
	return m_magnitude * lds->GetPosition().CalcFloatLength(); //MAGIC: Further from origin you move == stronger wind.
}


//--------------------------------------------------------------------------------------------------------------
Vector3f WormholeForce::CalcDirectionForState( const LinearDynamicsState * lds ) const
{
	return m_center - lds->GetPosition(); //MAGIC: Direction sends you back toward m_center.
}


//--------------------------------------------------------------------------------------------------------------
Vector3f WormholeForce::CalcForceForStateAndMass( const LinearDynamicsState* lds, float /*mass*/ ) const
{
	Vector3f windVector = CalcDirectionForState( lds ) * CalcMagnitudeForState( lds );
	Vector3f undampedWindForce = lds->GetVelocity() - windVector;

	return undampedWindForce * -m_dampedness;
}


//--------------------------------------------------------------------------------------------------------------
Vector3f SpringForce::CalcForceForStateAndMass( const LinearDynamicsState * lds, float /*mass*/ ) const
{
	Vector3f dampedVelocity = lds->GetVelocity() * -m_dampedness;
	Vector3f stiffenedPosition = lds->GetPosition() * -m_stiffness;

	return dampedVelocity + stiffenedPosition;
}





//--------------------------------------------------------------------------------------------------------------
Vector3f DebrisForce::CalcForceForStateAndMass( const LinearDynamicsState * lds, float mass ) const
{
	return CalcDirectionForState( lds ) * CalcMagnitudeForState( lds ) * mass;
}


//--------------------------------------------------------------------------------------------------------------
float DebrisForce::CalcMagnitudeForState( const LinearDynamicsState * lds ) const
{
	float upComponentForPosition = DotProduct( lds->GetPosition(), WORLD3D_UP );
	float upComponentForVelocity = DotProduct( lds->GetVelocity(), WORLD3D_UP );

	if ( upComponentForPosition < m_groundHeight )
		upComponentForPosition *= -10.f;
	else if ( upComponentForPosition > m_groundHeight && upComponentForVelocity < 0 )
		upComponentForPosition *= .65f;

	return upComponentForPosition;
}


//--------------------------------------------------------------------------------------------------------------
Vector3f DebrisForce::CalcDirectionForState( const LinearDynamicsState * lds ) const //The real problem: can't directly drag down velocity to zero.
{
	float upComponent = DotProduct( lds->GetPosition(), WORLD3D_UP );

	//If upComponent is negative, we're below ground and need to invert direction with slightly less magnitude.
	if ( upComponent < m_groundHeight )
		return WORLD3D_UP;
	else
		return WORLD3D_DOWN;
}
