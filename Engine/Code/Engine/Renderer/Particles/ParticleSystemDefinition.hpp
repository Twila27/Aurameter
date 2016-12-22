#pragma once


#include <vector>
#include "Engine/EngineCommon.hpp"
class ParticleEmitterDefinition;


class ParticleSystemDefinition
{
public:
	static ParticleSystemDefinition* Create( ResourceID id );

	ParticleEmitterDefinition* AddEmitter( ParticleEmitterDefinition* );
	const std::vector< ParticleEmitterDefinition* >& GetEmitters() const { return m_definedEmitters; }


private:
	ParticleSystemDefinition() {}
	ResourceID m_name;
	std::vector< ParticleEmitterDefinition* > m_definedEmitters;
};