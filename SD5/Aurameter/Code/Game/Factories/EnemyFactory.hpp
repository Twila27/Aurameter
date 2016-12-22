#pragma once


#include <map>
#include "Engine/Memory/UntrackedAllocator.hpp"


//-----------------------------------------------------------------------------
class Enemy;
class EnemyFactory;
struct XMLNode;
typedef std::string EnemyFactoryID;
typedef std::pair<EnemyFactoryID, EnemyFactory*> EnemyRegistryPair;
typedef std::map<EnemyFactoryID, EnemyFactory*, std::less<std::string>, UntrackedAllocator<EnemyRegistryPair> > EnemyRegistryMap;


//-----------------------------------------------------------------------------
class EnemyFactory
{
public:
	static void LoadAllFactories();
	static void CleanupFactories();
	static const EnemyRegistryMap& GetRegistry() { return s_enemyFactoryRegistry; }
	static Enemy* CreateEnemyFromName( const EnemyFactoryID& enemyName );


public:
	EnemyFactory( const XMLNode& enemyNode ) { PopulateFromXMLNode( enemyNode ); }
	void PopulateFromXMLNode( const XMLNode& enemyNode );
	~EnemyFactory();

	std::string GetName() const { return m_name; }
	Enemy* CreateEnemy();

private:
	std::string m_name;
	Enemy* m_templateEnemy;

	static EnemyRegistryMap s_enemyFactoryRegistry;
};