#pragma once


#include "Game/GameCommon.hpp"
#include "Engine/Memory/UntrackedAllocator.hpp"


//-----------------------------------------------------------------------------
class Sprite;
class Enemy;
class SpawnPattern;
class GameEntity;
class EngineEvent;
struct World;
struct XMLNode;


//-----------------------------------------------------------------------------
typedef std::vector< World*, UntrackedAllocator<World*> > ArenaRegistryVector;


//-----------------------------------------------------------------------------
struct World
{
public:
	static void LoadAllArenas();
	static void CloneRegistry( std::vector< World* >& outArenas ); 
	static void CleanupRegistry();


public:
	World( const XMLNode& arenaNode ) : m_currentWave( 0 ) { PopulateFromXMLNode( arenaNode ); }
	~World();
	void PopulateFromXMLNode( const XMLNode& arenaNode );
	int SpawnNextWave( std::vector<GameEntity*>& outEntityList, const WorldCoords2D& offsetFromOrigin = WorldCoords2D::ZERO );
	bool FinishTransition( EngineEvent* eventContext );
	bool HideBackground( EngineEvent* eventContext );
	bool ShowBackground( EngineEvent* eventContext );

	TerrainID m_terrain; //If looking for an m_name, that is this right now.

	Sprite* m_background;
	Sprite* m_transition;
	Sprite* m_titleCard;

	int m_currentWave;
	std::vector<SpawnPattern*> m_waves;


private:
	World( const World& other );
	static ArenaRegistryVector s_arenaTemplateRegistry; //Factory pattern affords retrying the game.
};