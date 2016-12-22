#include "Game/Factories/BulletFactory.hpp"
#include "Game/Entities/Bullet.hpp"

#include "Engine/FileUtils/FileUtils.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC BulletRegistryMap BulletFactory::s_bulletFactoryRegistry;


//--------------------------------------------------------------------------------------------------------------
void BulletFactory::LoadAllFactories()
{
	//Note we may have more than one Bullets node in a file.
	std::vector< std::string > m_bulletFiles = EnumerateFilesInDirectory( "Data/XML", "Bullets.xml" );

	for ( unsigned int bulletFileIndex = 0; bulletFileIndex < m_bulletFiles.size(); bulletFileIndex++ )
	{
		const char* xmlFilename = m_bulletFiles[ bulletFileIndex ].c_str();
		XMLNode bulletsRoot = XMLNode::openFileHelper( xmlFilename, "Bullets" );

		for ( int bulletIndex = 0; bulletIndex < bulletsRoot.nChildNode(); bulletIndex++ )
		{
			XMLNode bulletNode = bulletsRoot.getChildNode( bulletIndex );

			BulletFactory* newBulletFactory = new BulletFactory( bulletNode );
			std::string name = newBulletFactory->GetName();

			if ( s_bulletFactoryRegistry.find( name ) != s_bulletFactoryRegistry.end() )
				ERROR_AND_DIE( Stringf( "Found duplicate Bullet name %s in BulletFactory::LoadAllFactories!", name.c_str() ) );

			s_bulletFactoryRegistry.insert( BulletRegistryPair( name, newBulletFactory ) );
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
STATIC void BulletFactory::CleanupFactories()
{
	for ( const BulletRegistryPair& factoryPair : s_bulletFactoryRegistry ) 
		delete factoryPair.second;
	
	s_bulletFactoryRegistry.clear();
}


//--------------------------------------------------------------------------------------------------------------
void BulletFactory::PopulateFromXMLNode( const XMLNode& bulletNode )
{
	m_templateBullet = new Bullet( bulletNode );
	m_name = m_templateBullet->GetName();
}


//--------------------------------------------------------------------------------------------------------------
BulletFactory::~BulletFactory()
{
	delete m_templateBullet;
}


//--------------------------------------------------------------------------------------------------------------
Bullet* BulletFactory::CreateBullet()
{
	return m_templateBullet->GetClone();
}


//--------------------------------------------------------------------------------------------------------------
Bullet* BulletFactory::CreateBulletFromName( const BulletFactoryID& bulletName )
{
	BulletRegistryMap::iterator found = s_bulletFactoryRegistry.find( bulletName );
	if ( found == s_bulletFactoryRegistry.end() )
	{
		Logger::PrintfWithTag( "DEBUG", "Failed request %s sent to CreateBulletFromName!", bulletName.c_str() );
		return nullptr;
	}

	return found->second->CreateBullet();
}