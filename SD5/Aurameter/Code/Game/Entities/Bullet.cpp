#include "Game/Entities/Bullet.hpp"
#include "Engine/Physics/Forces.hpp"
#include "Engine/Physics/PhysicsUtils.hpp"


//--------------------------------------------------------------------------------------------------------------
Bullet* Bullet::GetClone() const
{
	return new Bullet( *this );
}


//--------------------------------------------------------------------------------------------------------------
Bullet::Bullet( const XMLNode& bulletNode )
	: GameEntity( ENTITY_TYPE_BULLET )
	, m_lifetimeSeconds( 5.f )
{
	m_timeLeftSeconds = m_lifetimeSeconds;
	m_rigidbody->AddForce( new ConstantWindForce( 5.f, Vector3f( WORLD2D_DOWN.x, WORLD2D_DOWN.y, 0.f ) ) );
	PopulateFromXMLNode( bulletNode );
}


//--------------------------------------------------------------------------------------------------------------
Bullet::Bullet( const Bullet& other )
	: GameEntity( other )
	, m_lifetimeSeconds( other.m_lifetimeSeconds )
	, m_timeLeftSeconds( other.m_timeLeftSeconds )
{

}


//--------------------------------------------------------------------------------------------------------------
void Bullet::PopulateFromXMLNode( const XMLNode& bulletNode )
{
	GameEntity::PopulateFromXMLNode( bulletNode );
}


//--------------------------------------------------------------------------------------------------------------
void Bullet::Update( float deltaSeconds )
{
	m_timeLeftSeconds -= deltaSeconds;

	GameEntity::Update( deltaSeconds );
}
