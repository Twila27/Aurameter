#include "Game/Factories/Pattern.hpp"
#include "Game/Factories/EnemyFactory.hpp"
#include "Game/Factories/BulletFactory.hpp"
#include "Game/Entities/GameEntity.hpp"
#include "Game/Entities/Enemy.hpp"
#include "Game/Entities/Bullet.hpp"
#include "Engine/Physics/PhysicsUtils.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC PatternRegistryMap SpawnPattern::s_patternRegistry;


//--------------------------------------------------------------------------------------------------------------
STATIC SpawnPattern* SpawnPattern::CreateOrGetPattern( const PatternID& patternName, const XMLNode& patternNode /*= XMLNode::emptyNode()*/ )
{
	PatternRegistryMap::iterator found = s_patternRegistry.find( patternName );
	if ( found != s_patternRegistry.end() )
		return found->second;

	SpawnPattern* newPattern = nullptr;
	if ( patternNode.isEmpty() )
	{
		newPattern = new SpawnPattern( "Uninitialized Pattern", nullptr, nullptr, 0 );
	}
	else
	{
		newPattern = CreateFromXMLNode( patternNode );
	}

	s_patternRegistry[ patternName ] = newPattern;
	return newPattern;
}


//--------------------------------------------------------------------------------------------------------------
STATIC SpawnPattern* SpawnPattern::CreateOneBulletPattern( const PatternID& patternName, const WorldCoords2D& patternOrigin, const XMLNode& singleSpawnNode )
{
	WorldCoords2D* spawnOffsets = new WorldCoords2D;
	spawnOffsets[ 0 ] = ReadXMLAttribute( singleSpawnNode, "offset", WorldCoords2D::ZERO );

	GameEntity** spawnedEntities = new GameEntity*;
	std::string entityFactoryName;
	entityFactoryName = ReadXMLAttribute( singleSpawnNode, "bullet", entityFactoryName );
	spawnedEntities[ 0 ] = BulletFactory::CreateBulletFromName( entityFactoryName );

	Vector2f additionalVelocity = Vector2f::ZERO;
	additionalVelocity = ReadXMLAttribute( singleSpawnNode, "velocity", additionalVelocity );
	if ( additionalVelocity != Vector2f::ZERO )
	{
		LinearDynamicsState* lds = spawnedEntities[ 0 ]->GetLinearDynamicsState();
		lds->SetVelocity( lds->GetVelocity() + Vector3f( additionalVelocity.x, additionalVelocity.y, 0.f ) );
	}

	SpawnPattern* newPattern = new SpawnPattern( patternName, spawnOffsets, spawnedEntities, 1, patternOrigin );
	s_patternRegistry[ patternName ] = newPattern;
	return newPattern;
}


//--------------------------------------------------------------------------------------------------------------
STATIC void SpawnPattern::CleanupRegistry()
{
	for ( const PatternRegistryPair& pair : s_patternRegistry )
		delete pair.second;

	s_patternRegistry.clear();
}


//--------------------------------------------------------------------------------------------------------------
SpawnPattern::SpawnPattern( const PatternID& name, WorldCoords2D* positions, GameEntity** entities, int numSpawns, const WorldCoords2D& patternOrigin /*= WorldCoords2D::ZERO*/ )
	: m_spawnOffsets( positions )
	, m_emittedEntities( entities )
	, m_numSpawns( numSpawns )
	, m_name( name )
	, m_patternOrigin( patternOrigin )
{
}


//--------------------------------------------------------------------------------------------------------------
SpawnPattern::SpawnPattern( const SpawnPattern& other )
	: m_numSpawns( other.m_numSpawns )
	, m_name( other.m_name )
	, m_patternOrigin( other.m_patternOrigin )
{
	m_spawnOffsets = new WorldCoords2D[ m_numSpawns ];
	m_emittedEntities = new GameEntity*[ m_numSpawns ];

	for ( int index = 0; index < m_numSpawns; index++ )
	{
		m_spawnOffsets[ index ] = other.m_spawnOffsets[ index ];
		m_emittedEntities[ index ] = other.m_emittedEntities[ index ]; //Does the assignment operator default to copy ctor at all?
	}
}


//--------------------------------------------------------------------------------------------------------------
SpawnPattern::~SpawnPattern()
{
	if ( m_spawnOffsets != nullptr )
	{
		delete m_spawnOffsets;
		m_spawnOffsets = nullptr;
	}

	if ( m_emittedEntities != nullptr )
	{
		delete m_emittedEntities;
		m_emittedEntities = nullptr;
	}
}


//--------------------------------------------------------------------------------------------------------------
void SpawnPattern::Instantiate( const WorldCoords2D& offsetFromPatternOrigin, std::vector<GameEntity*>& outEntityList )
{
	for ( int spawnIndex = 0; spawnIndex < m_numSpawns; spawnIndex++ )
	{
		GameEntity* clonedEntity = m_emittedEntities[ spawnIndex ]->GetClone(); //Note use of virtual overrides.
		clonedEntity->SetPosition( offsetFromPatternOrigin + m_patternOrigin + m_spawnOffsets[ spawnIndex ] );
		clonedEntity->Enable();
		outEntityList.push_back( clonedEntity );
	}
}


//--------------------------------------------------------------------------------------------------------------
STATIC SpawnPattern* SpawnPattern::CreateFromXMLNode( const XMLNode& patternNode )
{
	std::string patternName;
	patternName = ReadXMLAttribute( patternNode, "name", patternName );

	WorldCoords2D patternOrigin = WorldCoords2D::ZERO;
	patternOrigin = ReadXMLAttribute( patternNode, "origin", patternOrigin );

	int numSpawns = patternNode.nChildNode( "Spawn" );
	int numSpawnsMinusDelayedBullets = numSpawns;
	for ( int spawnIndex = 0; spawnIndex < numSpawns; spawnIndex++ )
	{
		XMLNode spawnNode = patternNode.getChildNode( "Spawn", spawnIndex );
		if ( spawnNode.isAttributeSet( "spawnDelay" ) )
		{
			--numSpawnsMinusDelayedBullets;
			continue; //Already handled in Enemy's CreateFromXMLNode as a separate pattern.
		}
	}

	WorldCoords2D* spawnOffsets = new WorldCoords2D[ numSpawnsMinusDelayedBullets ];
	GameEntity** spawnedEntities = new GameEntity*[ numSpawnsMinusDelayedBullets ];

	int numDelayedBulletsSkipped = 0;
	for ( int spawnIndex = 0; spawnIndex < numSpawns; spawnIndex++ )
	{
		int tightlyPackedIndex = spawnIndex - numDelayedBulletsSkipped;

		XMLNode spawnNode = patternNode.getChildNode( "Spawn", spawnIndex );
		if ( spawnNode.isAttributeSet( "spawnDelay" ) )
		{
			++numDelayedBulletsSkipped;
			continue; //Already handled in Enemy's CreateFromXMLNode as a separate pattern.
		}

		spawnOffsets[ tightlyPackedIndex ] = ReadXMLAttribute( spawnNode, "offset", WorldCoords2D::ZERO );

		std::string entityFactoryName;
		if ( spawnNode.isAttributeSet( "enemy" ) )
		{
			entityFactoryName = ReadXMLAttribute( spawnNode, "enemy", entityFactoryName );
			spawnedEntities[ tightlyPackedIndex ] = EnemyFactory::CreateEnemyFromName( entityFactoryName );
		}
		else if ( spawnNode.isAttributeSet( "bullet" ) )
		{
			entityFactoryName = ReadXMLAttribute( spawnNode, "bullet", entityFactoryName );
			spawnedEntities[ tightlyPackedIndex ] = BulletFactory::CreateBulletFromName( entityFactoryName );

			Vector2f additionalVelocity = Vector2f::ZERO;
			additionalVelocity = ReadXMLAttribute( spawnNode, "velocity", additionalVelocity );
			if ( additionalVelocity != Vector2f::ZERO )
			{
				LinearDynamicsState* lds = spawnedEntities[ tightlyPackedIndex ]->GetLinearDynamicsState();
				lds->SetVelocity( lds->GetVelocity() + Vector3f( additionalVelocity.x, additionalVelocity.y, 0.f ) );
			}
		}
		else ERROR_RECOVERABLE( "Failed to find anything to spawn!" );
	}

	return new SpawnPattern( patternName, spawnOffsets, spawnedEntities, numSpawnsMinusDelayedBullets, patternOrigin );
}
