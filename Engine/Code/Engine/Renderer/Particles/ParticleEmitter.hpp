#pragma once


#include <vector>
#include "Engine/Renderer/Particles/Particle.hpp"
#include "Engine/Renderer/RenderState.hpp"
#include "Engine/Renderer/Mesh.hpp"


//-----------------------------------------------------------------------------
class ParticleEmitterDefinition;
class Material;
class MeshRenderer;



//-----------------------------------------------------------------------------
class ParticleEmitter
{
public:
	static ParticleEmitter* Create( ParticleEmitterDefinition* defn, const Vector2f& systemPosition );
	~ParticleEmitter();


public:

	bool IsLooping() const;
	bool IsExpired() const;
	unsigned int GetNumLiveParticles() const { return m_particles.size(); }

	void Update( float deltaSeconds );
	void Render();
	void MarkForDeletion();


private:
	ParticleEmitter();

	int FillMesh( Particle* baseAddress, unsigned int numToAdd );

	float m_secondsSinceLastSpawn;
	bool m_isMarkedForDeletion;
	int m_numInitialSpawnsLeft;
	Vector2f m_position;
	ParticleEmitterDefinition const* m_emitterDefinition;
	std::vector<Particle> m_particles;

	std::shared_ptr<Mesh> m_particleBatchMesh;
	MeshRenderer* m_emitterMeshRenderer; //Material's kept on here until needs change.
};
