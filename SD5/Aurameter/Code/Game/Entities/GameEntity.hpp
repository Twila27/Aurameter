#pragma once


#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Rgba.hpp"
#include "Engine/Core/Entity.hpp"


//-----------------------------------------------------------------------------
class Sprite;
class Material;
class LinearDynamicsState;
struct XMLNode;


//-----------------------------------------------------------------------------
class GameEntity : public Entity
{
public:
	GameEntity() {} //For Pattern to create an uninitialized array.
	GameEntity( EntityType entityType );
	GameEntity( const GameEntity& other ); //To deep copy the Sprite*.
	GameEntity( EntityType entityType, ResourceID spriteName, const WorldCoords2D& pivotPosition, const Rgba& tint = Rgba::WHITE );
	virtual GameEntity* GetClone() const { throw new std::logic_error( "Only implemented for entities spawned in patterns: Enemy, Bullet." ); }
	virtual ~GameEntity();

	int GetHealth() const { return m_health; }
	int GetMaxHealth() const { return m_maxHealth; }
	WorldCoords2D GetPosition() const;
	LinearDynamicsState* GetLinearDynamicsState() { return m_rigidbody; }
	void Enable();
	Sprite* GetSprite() const { return m_sprite; }
	std::string GetName() const { return m_name; }
	bool HasBeenDeflected() const { return m_hasBeenDeflected; }

	void SetPosition( const WorldCoords2D& newPosition );
	void SetHasBeenDeflected( bool newVal ) { m_hasBeenDeflected = newVal; }
	void SetMaterial( Material* newMat );

	bool IsEntityType( EntityType entityType ) { return m_entityType == entityType; }
	EntityType GetEntityType() const { return m_entityType; }
	virtual void Update( float deltaSeconds );
	virtual bool IsExpired() { return m_health <= 0; }
	virtual bool IsPlayer() { return false; }

	bool AddHealthDelta( int delta );
	bool SubtractHealthDelta( int delta );

	static const EntityID s_INVALID_ID;


protected:
	void PopulateFromXMLNode( const XMLNode& entityNode );

	int m_health;
	int m_maxHealth;
	EntityType m_entityType;
	std::string m_name;
	EntityID m_entityID;
	Sprite* m_sprite;
	LinearDynamicsState* m_rigidbody;


private:
	bool m_hasBeenDeflected; //Skips repeat deflections.
	static EntityID s_BASE_ENTITY_ID;
};
