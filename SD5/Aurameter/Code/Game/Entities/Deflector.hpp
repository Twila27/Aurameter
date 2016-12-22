#pragma once


#include "Game/Entities/GameEntity.hpp"
#include "Engine/Memory/UntrackedAllocator.hpp"
#include <map>


//-----------------------------------------------------------------------------
class LinearDynamicsState;
class Player;
class Deflector;


//-----------------------------------------------------------------------------
typedef void( DeflectorCallback )( LinearDynamicsState* deflectedBulletState );
typedef std::pair< TerrainID, Deflector* > DeflectorRegistryPair;
typedef std::map< TerrainID, Deflector*, std::less<TerrainID>, UntrackedAllocator<DeflectorRegistryPair> > DeflectorRegistryMap;


//-----------------------------------------------------------------------------
class Deflector : public GameEntity
{
public:
	static void LoadAllDeflectors();
	static std::string GetDeflectorDescForTerrain( TerrainID terrain );
	static Deflector* GetDeflectorCloneForTerrain( TerrainID terrain ) { return new Deflector( *s_deflectorRegistry[ terrain ] ); }
	static void CleanupRegistry();

	Deflector( const std::string& name, DeflectorCallback* cb, const Player* player );
	Deflector( const Deflector& other );

	virtual void Update( float deltaSeconds );
	void Launch();
	void SetPlayer( const Player* player ); //For late bind.
	virtual bool IsExpired() override;
	DeflectorCallback* m_deflectLogic;


private:
	const Player* m_player;
	bool m_hasLaunched;

	static void RegisterDeflector( const std::string& deflectorName, const TerrainID& terrain, const Rgba& tint, DeflectorCallback* deflectLogic );
	static const Vector2f s_MAX_RADIUS;
	static const float s_EXPANSION_RATE;
	static DeflectorRegistryMap s_deflectorRegistry;
};