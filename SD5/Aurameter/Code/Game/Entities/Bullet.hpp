#pragma once


#include "Game/Entities/GameEntity.hpp"


//-----------------------------------------------------------------------------
struct XMLNode;


//-----------------------------------------------------------------------------
class Bullet : public GameEntity
{
public:
	Bullet( const Bullet& other );
	Bullet( const XMLNode& bulletNode );
	virtual Bullet* GetClone() const override;
	void PopulateFromXMLNode( const XMLNode& bulletNode );

	Bullet( const WorldCoords2D& pos, const Vector2f& vel = Vector2f::ZERO );
	virtual void Update( float deltaSeconds ) override;
	virtual bool IsExpired() override { return GameEntity::IsExpired() || m_timeLeftSeconds <= 0.f; }


private:
	float m_timeLeftSeconds;
	float m_lifetimeSeconds;
};
