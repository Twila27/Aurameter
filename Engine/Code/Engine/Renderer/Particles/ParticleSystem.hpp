#pragma once


#include <vector>
#include "Engine/EngineCommon.hpp"


//-----------------------------------------------------------------------------
class ParticleSystemDefinition;
class ParticleEmitter;



//-----------------------------------------------------------------------------
class ParticleSystem
{
public:
	static ParticleSystemDefinition* Register( ResourceID uniqueSystemName ); //For defining.
	static void Play( ResourceID existingSystemName, RenderLayerID layerID, const Vector2f& systemPosition ); //For non-looping fire-forget.
	static ParticleSystem* Create( ResourceID existingSystemName, RenderLayerID layerID, const Vector2f& systemPosition ); //For persistent handles to systems.
	static void Destroy( ParticleSystem* );


public:

	bool IsLooping() const;
	bool IsExpired() const;
	unsigned int GetNumLiveParticles() const;

	void Update( float deltaSeconds );
	void Render();


private:
	ParticleSystem() {}
	~ParticleSystem();

	ParticleSystemDefinition const* m_systemDefinition;
	std::vector< ParticleEmitter* > m_emitters;
};
