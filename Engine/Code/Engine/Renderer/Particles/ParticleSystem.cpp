#include "Engine/Renderer/Particles/ParticleSystem.hpp"

#include "Engine/Renderer/Particles/ParticleSystemDefinition.hpp"
#include "Engine/Renderer/Particles/ParticleEmitter.hpp"
#include "Engine/Renderer/ResourceDatabase.hpp"
#include "Engine/Renderer/SpriteRenderer.hpp"


//--------------------------------------------------------------------------------------------------------------
ParticleSystem::~ParticleSystem()
{
	for ( ParticleEmitter* emitter : m_emitters )
		delete emitter;
}


//--------------------------------------------------------------------------------------------------------------
STATIC ParticleSystemDefinition* ParticleSystem::Register( ResourceID uniqueSystemName )
{
	//Returns an empty definition that AddEmitter( ParticleEmitterDefn* ) can be called on.
	ParticleSystemDefinition* psd = ParticleSystemDefinition::Create( uniqueSystemName );
	ResourceDatabase::Instance()->AddParticleSystem( uniqueSystemName, psd );
	return psd;
}


//--------------------------------------------------------------------------------------------------------------
STATIC void ParticleSystem::Play( ResourceID existingSystemName, RenderLayerID layerID, const Vector2f& systemPosition )
{
	ParticleSystem* ps = new ParticleSystem();
	ps->m_systemDefinition = ResourceDatabase::Instance()->GetParticleSystemDefinition( existingSystemName );

	const std::vector< ParticleEmitterDefinition* >& emitterDefns = ps->m_systemDefinition->GetEmitters();
	for ( ParticleEmitterDefinition* emitterDefn : emitterDefns )
	{
		ps->m_emitters.push_back( ParticleEmitter::Create( emitterDefn, systemPosition ) );
		ASSERT_OR_DIE( !ps->m_emitters.back()->IsLooping(), "ParticleSystem::Play only works with non-looping systems!" );
	}

	SpriteRenderer::CreateOrGetLayer( layerID )->AddParticleSystem( ps );
}


//--------------------------------------------------------------------------------------------------------------
STATIC ParticleSystem* ParticleSystem::Create( ResourceID existingSystemName, RenderLayerID layerID, const Vector2f& systemPosition )
{
	ParticleSystem* ps = new ParticleSystem();
	ps->m_systemDefinition = ResourceDatabase::Instance()->GetParticleSystemDefinition( existingSystemName );

	const std::vector< ParticleEmitterDefinition* >& emitterDefns = ps->m_systemDefinition->GetEmitters();
	for ( ParticleEmitterDefinition* emitterDefn : emitterDefns ) 
	{
		ps->m_emitters.push_back( ParticleEmitter::Create( emitterDefn, systemPosition ) );
		ASSERT_OR_DIE( ps->m_emitters.back()->IsLooping(), "ParticleSystem::Create only works with looping systems!" );
	}

	SpriteRenderer::CreateOrGetLayer( layerID )->AddParticleSystem( ps );

	return ps;
}


//--------------------------------------------------------------------------------------------------------------
STATIC void ParticleSystem::Destroy( ParticleSystem* particleSystem )
{
	for ( ParticleEmitter* emitter : particleSystem->m_emitters )
		emitter->MarkForDeletion();
}


//--------------------------------------------------------------------------------------------------------------
bool ParticleSystem::IsExpired() const
{
	for ( ParticleEmitter* emitter : m_emitters )
		if ( !emitter->IsExpired() )
			return false;

	return true;
}


//--------------------------------------------------------------------------------------------------------------
unsigned int ParticleSystem::GetNumLiveParticles() const
{
	unsigned int total = 0;

	for ( ParticleEmitter* emitter : m_emitters )
		total += emitter->GetNumLiveParticles();

	return total;
}


//--------------------------------------------------------------------------------------------------------------
bool ParticleSystem::IsLooping() const
{
	for ( ParticleEmitter* emitter : m_emitters )
		if ( emitter->IsLooping() )
			return true;

	return false;
}


//--------------------------------------------------------------------------------------------------------------
void ParticleSystem::Update( float deltaSeconds )
{
	for ( ParticleEmitter* emitter : m_emitters )
		emitter->Update( deltaSeconds );
}


//--------------------------------------------------------------------------------------------------------------
void ParticleSystem::Render()
{
	for ( ParticleEmitter* emitter : m_emitters )
		emitter->Render();
}
