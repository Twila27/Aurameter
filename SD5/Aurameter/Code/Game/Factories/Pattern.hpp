#pragma once


#include "Engine/Memory/UntrackedAllocator.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"
#include "Game/GameCommon.hpp"
#include <map>


//-----------------------------------------------------------------------------
class SpawnPattern;
class GameEntity;
typedef std::string PatternID;
typedef std::pair< PatternID, SpawnPattern* > PatternRegistryPair;
typedef std::map< PatternID, SpawnPattern*, std::less<PatternID>, UntrackedAllocator<PatternRegistryPair> > PatternRegistryMap;


//-----------------------------------------------------------------------------
class SpawnPattern
{
public:
	static SpawnPattern* CreateOrGetPattern( const PatternID& patternName, const XMLNode& patternNode = XMLNode::emptyNode() );
	static SpawnPattern* CreateOneBulletPattern( const PatternID& patternName, const WorldCoords2D& patternOrigin, const XMLNode& singleSpawnNode );
	static void CleanupRegistry();
	static SpawnPattern* CreateFromXMLNode( const XMLNode& patternNode );

	SpawnPattern( const PatternID& name, WorldCoords2D* positions, GameEntity** entities, int numSpawns, const WorldCoords2D& patternOrigin = WorldCoords2D::ZERO );
	SpawnPattern( const SpawnPattern& other );
	~SpawnPattern();
	void Instantiate( const WorldCoords2D& patternOrigin, std::vector<GameEntity*>& outEntityList );
	
	PatternID m_name;
	WorldCoords2D m_patternOrigin;
	WorldCoords2D* m_spawnOffsets; //Relative to pattern center supplied on Instantiate().
	GameEntity** m_emittedEntities;
	int m_numSpawns;

private:
	static PatternRegistryMap s_patternRegistry;
};
