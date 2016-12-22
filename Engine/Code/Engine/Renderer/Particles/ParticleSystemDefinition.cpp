#include "Engine/Renderer/Particles/ParticleSystemDefinition.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC ParticleSystemDefinition* ParticleSystemDefinition::Create( ResourceID id )
{
	ParticleSystemDefinition* psd = new ParticleSystemDefinition();
	psd->m_name = id;
	return psd;
}


//--------------------------------------------------------------------------------------------------------------
ParticleEmitterDefinition* ParticleSystemDefinition::AddEmitter( ParticleEmitterDefinition* defn )
{
	m_definedEmitters.push_back( defn );
	return m_definedEmitters.back();
}
