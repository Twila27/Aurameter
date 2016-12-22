#pragma once


#include <map>
#include "Engine/Memory/UntrackedAllocator.hpp"


//-----------------------------------------------------------------------------
class Bullet;
class BulletFactory;
struct XMLNode;
typedef std::string BulletFactoryID;
typedef std::pair<BulletFactoryID, BulletFactory*> BulletRegistryPair;
typedef std::map<BulletFactoryID, BulletFactory*, std::less<std::string>, UntrackedAllocator<BulletRegistryPair> > BulletRegistryMap;


//-----------------------------------------------------------------------------
class BulletFactory
{
public:
	static void LoadAllFactories();
	static void CleanupFactories();
	static const BulletRegistryMap& GetRegistry() { return s_bulletFactoryRegistry; }
	static Bullet* CreateBulletFromName( const BulletFactoryID& name );


public:
	BulletFactory( const XMLNode& bulletNode ) { PopulateFromXMLNode( bulletNode ); }
	void PopulateFromXMLNode( const XMLNode& bulletNode );
	~BulletFactory();
	
	std::string GetName() const { return m_name; }
	Bullet* CreateBullet();

private:
	std::string m_name;
	Bullet* m_templateBullet;

	static BulletRegistryMap s_bulletFactoryRegistry;
};
