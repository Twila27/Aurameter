#pragma once


#include "Game/Entities/Agent.hpp"
#include <vector>


//-----------------------------------------------------------------------------
class Deflector;
class ParticleSystem;


//-----------------------------------------------------------------------------
class Player : public Agent
{
public:
	Player();
	virtual ~Player() override;
	
	virtual bool IsPlayer() { return true; }
	virtual void Update( float deltaSeconds ) override;

	bool IsGeomancing() const { return m_isGeomancing; }
	int GetAurameterValue() const { return m_aurameterValue; }

	Deflector* AddDeflector( TerrainID currentTerrain );
	void AddAurameterDelta( EntityType entityType );

	ParticleSystem* m_smokeParticleSystem;


private:
	void UpdateFromKeyboard( float deltaSeconds );
	void UpdateFromXInput( float deltaSeconds );

	std::vector<Deflector*> m_shields;
	int m_aurameterValue;
	bool m_isGeomancing;


	static const float s_PLAYER_MOVEMENT_SPEED;
};
