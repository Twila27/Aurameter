#include "Game/Entities/Enemy.hpp"

#include "Engine/FileUtils/XMLUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/TheEventSystem.hpp"

#include "Game/Factories/Pattern.hpp"


//--------------------------------------------------------------------------------------------------------------
void Enemy::Update( float deltaSeconds )
{
	m_secondsAlive += deltaSeconds;

	const float SECONDS_UNTIL_LOOP = 2.5f;
	if ( m_secondsAlive > m_firingPatterns.rbegin()->first + SECONDS_UNTIL_LOOP )
	{
		m_lastPattern = 0;
		m_secondsAlive = 0.f;
	}

	//Get map at right time...
	int numIteration = 0;
	for ( auto patternIter = m_firingPatterns.begin(); patternIter != m_firingPatterns.end(); ++patternIter )
	{
		if ( numIteration++ < m_lastPattern )
			continue;

		if ( patternIter->first > m_secondsAlive )
			return;

		++m_lastPattern;
		GameEventPatternFired ev;
		patternIter->second->Instantiate( this->GetPosition(), ev.m_firedBullets );
		TheEventSystem::Instance()->TriggerEvent( "OnEnemyPatternFired", &ev );
	}
}

	
//--------------------------------------------------------------------------------------------------------------
Enemy* Enemy::GetClone() const
{
	return new Enemy( *this );
}


//--------------------------------------------------------------------------------------------------------------
void Enemy::PopulateFromXMLNode( const XMLNode& enemyNode )
{
	Agent::PopulateFromXMLNode( enemyNode );

	int numWaves = enemyNode.nChildNode( "Pattern" );

	for ( int waveIndex = 0; waveIndex < numWaves; waveIndex++ )
	{
		XMLNode patternNode = enemyNode.getChildNode( "Pattern" );

		std::string patternName;
		patternName = ReadXMLAttribute( patternNode, "name", patternName );

		Vector2f initialVelocity = Vector2f::ZERO;
		initialVelocity = ReadXMLAttribute( patternNode, "velocity", initialVelocity );

		float whenToSpawnSeconds = 1.f;
		whenToSpawnSeconds = ReadXMLAttribute( patternNode, "whenToSpawn", whenToSpawnSeconds );

		//Break out into multiple patterns if there's a delay, not sure how else to handle the per-bullet spawn delay.
		bool doesDelayedSpawnExist = ( patternNode.getChildNodeWithAttribute( "Spawn", "spawnDelay" ).isEmpty() > 0 ) ? false : true;
		int numBulletSpawns = patternNode.nChildNode( "Spawn" );
		int numOneBulletPatterns = 0;
		if ( doesDelayedSpawnExist )
		{
			Vector2f patternOrigin = Vector2f::ZERO;
			patternOrigin = ReadXMLAttribute( patternNode, "origin", patternOrigin );

			int delayedBulletSpawnIndex = 0; //Written to by below call.
			XMLNode delayedBulletSpawnNode = patternNode.getChildNodeWithAttribute( "Spawn", "spawnDelay", 0, &delayedBulletSpawnIndex );

			do
			{
				SpawnPattern* newOneBulletPattern = SpawnPattern::CreateOneBulletPattern( Stringf( "%s - OneBulletPattern #%d Index %d", patternName.c_str(), numOneBulletPatterns, delayedBulletSpawnIndex ), patternOrigin, delayedBulletSpawnNode );

				float spawnDelaySeconds = 0.f;
				spawnDelaySeconds = ReadXMLAttribute( delayedBulletSpawnNode, "spawnDelay", spawnDelaySeconds );

				m_firingPatterns.insert( TimeOrderedMapPair( whenToSpawnSeconds + spawnDelaySeconds, newOneBulletPattern ) );

				++numOneBulletPatterns;

				delayedBulletSpawnNode = patternNode.getChildNodeWithAttribute( "Spawn", "spawnDelay", 0, &delayedBulletSpawnIndex );
			} 
			while ( !delayedBulletSpawnNode.isEmpty() );
		}

		ASSERT_OR_DIE( numBulletSpawns > numOneBulletPatterns, "Need at least one non-delayed bullet in a firing pattern!" );
			//Could surround below call with an if-block instead, but it really doesn't make sense if no bullets are fired at pattern's "start".
		m_firingPatterns.insert( TimeOrderedMapPair( whenToSpawnSeconds, SpawnPattern::CreateOrGetPattern( patternName, patternNode ) ) );
	}
}
