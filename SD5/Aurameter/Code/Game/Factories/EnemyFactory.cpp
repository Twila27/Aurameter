#include "Game/Factories/EnemyFactory.hpp"
#include "Game/Entities/Enemy.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/FileUtils/FileUtils.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC EnemyRegistryMap EnemyFactory::s_enemyFactoryRegistry;


//--------------------------------------------------------------------------------------------------------------
void EnemyFactory::LoadAllFactories()
{
	//Note we may have more than one Enemies node in a file.
	std::vector< std::string > m_enemyFiles = EnumerateFilesInDirectory( "Data/XML", "Enemies.xml" );

	for ( unsigned int enemyFileIndex = 0; enemyFileIndex < m_enemyFiles.size(); enemyFileIndex++ )
	{
		const char* xmlFilename = m_enemyFiles[ enemyFileIndex ].c_str();
		XMLNode enemiesRoot = XMLNode::openFileHelper( xmlFilename, "Enemies" );

		for ( int enemyIndex = 0; enemyIndex < enemiesRoot.nChildNode(); enemyIndex++ )
		{
			XMLNode enemyNode = enemiesRoot.getChildNode( enemyIndex );

			EnemyFactory* newEnemyFactory = new EnemyFactory( enemyNode );
			std::string name = newEnemyFactory->GetName();

			if ( s_enemyFactoryRegistry.find( name ) != s_enemyFactoryRegistry.end() )
				ERROR_AND_DIE( Stringf( "Found duplicate Enemy name %s in EnemyFactory::LoadAllFactories!", name.c_str() ) );

			s_enemyFactoryRegistry.insert( EnemyRegistryPair( name, newEnemyFactory ) );
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
STATIC void EnemyFactory::CleanupFactories()
{
	for ( const EnemyRegistryPair& factoryPair : s_enemyFactoryRegistry )
		delete factoryPair.second;

	s_enemyFactoryRegistry.clear();
}


//--------------------------------------------------------------------------------------------------------------
Enemy* EnemyFactory::CreateEnemyFromName( const EnemyFactoryID& enemyName )
{
	EnemyRegistryMap::iterator found = s_enemyFactoryRegistry.find( enemyName );
	if ( found == s_enemyFactoryRegistry.end() )
	{
		Logger::PrintfWithTag( "DEBUG", "Failed request %s sent to CreateEnemyFromName!", enemyName.c_str() );
		return nullptr;
	}

	return found->second->CreateEnemy();
}


//--------------------------------------------------------------------------------------------------------------
void EnemyFactory::PopulateFromXMLNode( const XMLNode& enemyNode )
{
	m_templateEnemy = new Enemy( enemyNode );
	m_name = m_templateEnemy->GetName();
}


//--------------------------------------------------------------------------------------------------------------
EnemyFactory::~EnemyFactory()
{
	delete m_templateEnemy;
}


//--------------------------------------------------------------------------------------------------------------
Enemy* EnemyFactory::CreateEnemy()
{
	return m_templateEnemy->GetClone();
}
