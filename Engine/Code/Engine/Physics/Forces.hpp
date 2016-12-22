#pragma once


#include "Engine/EngineCommon.hpp"


//-----------------------------------------------------------------------------
class LinearDynamicsState;


//-----------------------------------------------------------------------------
class Force
{
public:

	virtual Vector3f CalcForceForStateAndMass( const LinearDynamicsState* lds, float mass ) const = 0;
	virtual Force* GetCopy() const = 0;


protected:

	Force( float magnitude, const Vector3f& direction = Vector3f::ZERO )
		: m_magnitude( magnitude )
		, m_direction( direction )
	{
	}

	float m_magnitude;
	Vector3f m_direction;

	virtual float CalcMagnitudeForState( const LinearDynamicsState* /*lds*/ ) const { return m_magnitude; }
	virtual Vector3f CalcDirectionForState( const LinearDynamicsState* /*lds*/ ) const { return m_direction; }
};


//-----------------------------------------------------------------------------
struct GravityForce : public Force // m*g
{
	GravityForce()
		: Force( 9.81f, WORLD3D_DOWN )
	{
	}
	GravityForce( float magnitude, const Vector3f& direction = WORLD3D_DOWN )
		: Force( magnitude, direction )
	{
	}

	Vector3f CalcForceForStateAndMass( const LinearDynamicsState* lds, float mass ) const override;
	Force* GetCopy() const { return new GravityForce( *this ); }
};


//-----------------------------------------------------------------------------
struct DebrisForce : public Force //The real problem: can't directly drag down velocity to zero, as lds is const.
{
	DebrisForce()
		: Force( 9.81f, WORLD3D_DOWN ), m_groundHeight( 0.f )
	{
	}
	DebrisForce( float magnitude, float groundHeight = 0.f, const Vector3f& direction = WORLD3D_DOWN )
		: Force( magnitude, direction ), m_groundHeight( groundHeight )
	{
	}

	float m_groundHeight;

	Vector3f CalcForceForStateAndMass( const LinearDynamicsState* lds, float mass ) const override;
	float CalcMagnitudeForState( const LinearDynamicsState* lds ) const override; //Magnitude shrinks if you hit/sink below ground.
	virtual Vector3f CalcDirectionForState( const LinearDynamicsState* lds ) const override; //Direction inverts if you hit/sink below ground.
	Force* GetCopy() const { return new DebrisForce( *this ); }
};


//-----------------------------------------------------------------------------
struct ConstantWindForce : public Force // -c*(v - w)
{
	ConstantWindForce( float magnitude, const Vector3f& direction, float dampedness = 1.0f )
		: Force( magnitude, direction )
		, m_dampedness( dampedness )
	{
	}

	float m_dampedness; //"c".

	Vector3f CalcForceForStateAndMass( const LinearDynamicsState* lds, float mass ) const override;
	Force* GetCopy() const { return new ConstantWindForce( *this ); }
};


//-----------------------------------------------------------------------------
struct WormholeForce : public Force // -c*(v - w(pos))
{
	WormholeForce( Vector3f center, float magnitude, const Vector3f& direction, float dampedness = 1.0f )
		: Force( magnitude, direction )
		, m_dampedness( dampedness )
		, m_center( center )
	{
	}

	float m_dampedness; //"c".
	Vector3f m_center;

	//Overriding to make wind = wind(pos).
	float CalcMagnitudeForState( const LinearDynamicsState* lds ) const override; //Further from origin you move == stronger the wind.
	virtual Vector3f CalcDirectionForState( const LinearDynamicsState* lds ) const override; //Direction sends you back toward origin.
	Vector3f CalcForceForStateAndMass( const LinearDynamicsState* lds, float mass ) const override;
	Force* GetCopy() const { return new WormholeForce( *this );	}
};


//-----------------------------------------------------------------------------
struct SpringForce : public Force // -cv + -kx
{
	SpringForce( float magnitude, const Vector3f& direction, float stiffness, float dampedness = 1.0f )
		: Force( magnitude, direction )
		, m_dampedness( dampedness )
		, m_stiffness( stiffness )
	{
	}

	float m_dampedness; //"c".
	float m_stiffness; //"k".

	Vector3f CalcForceForStateAndMass( const LinearDynamicsState* lds, float mass ) const override;
	Force* GetCopy() const { return new SpringForce( *this ); }
};
