#pragma once


#include "Game/Entities/Agent.hpp"
#include "Engine/Core/EngineEvent.hpp"


//-----------------------------------------------------------------------------
struct XMLNode;
class SpawnPattern;
typedef std::multimap< float, SpawnPattern* > TimeOrderedMap;
typedef std::pair< float, SpawnPattern* > TimeOrderedMapPair;


//-----------------------------------------------------------------------------
struct GameEventPatternFired : public EngineEvent
{
	GameEventPatternFired() : EngineEvent( "OnEnemyPatternFired" ) {}
	std::vector<GameEntity*> m_firedBullets;
};


//-----------------------------------------------------------------------------
class Enemy : public Agent
{
public:
	Enemy( const XMLNode& enemyNode ) 
		: Agent( ENTITY_TYPE_ENEMY ) 
		, m_secondsAlive( 0.f )
		, m_lastPattern( 0 )
	{ 
		PopulateFromXMLNode( enemyNode ); 
	}
	virtual void Update( float deltaSeconds ) override;
	virtual Enemy* GetClone() const override;
	void PopulateFromXMLNode( const XMLNode& enemyNode ); 

private:
	TimeOrderedMap m_firingPatterns; //Does not need a deep copy since it's clone-and-forget usage.
	int m_lastPattern;
	float m_secondsAlive;
};
