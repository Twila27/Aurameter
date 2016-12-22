#pragma once

#include "Engine/Memory/UntrackedAllocator.hpp"
#include "Engine/Renderer/SpriteResource.hpp"
#include <map>


//-----------------------------------------------------------------------------
class ResourceDatabase;
class SpriteResource;
class AnimatedSpriteSequence;
class ParticleSystemDefinition;
class ParticleEmitterDefinition;


//-----------------------------------------------------------------------------
typedef std::pair< ResourceID, SpriteResource* > SpriteResourceRegistryPair;
typedef std::map< ResourceID, SpriteResource*, std::less<ResourceID>, UntrackedAllocator<SpriteResourceRegistryPair> > SpriteResourceRegistryMap;
typedef std::pair< ResourceID, AnimatedSpriteSequence* > SpriteSequenceRegistryPair;
typedef std::map< ResourceID, AnimatedSpriteSequence*, std::less<ResourceID>, UntrackedAllocator<SpriteSequenceRegistryPair> > SpriteSequenceRegistryMap;

typedef std::pair< ResourceID, ParticleSystemDefinition* > ParticleSystemDefinitionRegistryPair;
typedef std::map< ResourceID, ParticleSystemDefinition*, std::less<ResourceID>, UntrackedAllocator<ParticleSystemDefinitionRegistryPair> > ParticleSystemDefinitionRegistryMap;
typedef std::pair< ResourceID, ParticleEmitterDefinition* > ParticleEmitterDefinitionRegistryPair;
typedef std::map< ResourceID, ParticleEmitterDefinition*, std::less<ResourceID>, UntrackedAllocator<ParticleEmitterDefinitionRegistryPair> > ParticleEmitterDefinitionRegistryMap;


//-----------------------------------------------------------------------------
enum ResourceType
{
	RESOURCE_TYPE_SPRITE,
	RESOURCE_TYPE_SPRITE_ANIMATION,
	RESOURCE_TYPE_PARTICLE_SYSTEM,
	RESOURCE_TYPE_PARTICLE_EMITTER,
	NUM_RESOURCE_TYPES
};
extern ResourceType GetResourceTypeForString( const std::string& typeName );
extern std::string GetStringForResourceType( ResourceType type );


//-----------------------------------------------------------------------------
TODO( "MOVE OUT OF RENDERER INTO CORE FILTER" );
class ResourceDatabase
{
public:
	static void LoadAll();
	static void LoadAllSpriteResources();
	static void ReloadAllSpriteResources();
	static void WriteSpritesToFile( const std::string& filename );
	static void Shutdown() { delete s_theResourceDatabase; s_theResourceDatabase = nullptr; }

	static ResourceDatabase* /*CreateOrGet*/Instance();
	static unsigned int Count( ResourceID id ) { return s_spriteResourceRegistry.count( id ); }

	//Sprite resources and animated sprite sequences.
	static const SpriteResource* GetSpriteResource( ResourceID id ) { return s_spriteResourceRegistry.at( id ); }
	static SpriteResource* EditSpriteResource( ResourceID id ) { return s_spriteResourceRegistry.at( id ); }
	static const AnimatedSpriteSequence* GetSpriteSequence( ResourceID id ) { return s_spriteSequenceRegistry.at( id ); }
	static AnimatedSpriteSequence* EditSpriteSequence( ResourceID id ) { return s_spriteSequenceRegistry.at( id ); }

	//Particle system resources.
	static const ParticleSystemDefinition* GetParticleSystemDefinition( ResourceID id ) { return s_particleSystemRegistry.at( id ); }
	static ParticleSystemDefinition*  EditParticleSystemDefinition( ResourceID id ) { return s_particleSystemRegistry.at( id ); }
	static const ParticleEmitterDefinition* GetParticleEmitterDefinition( ResourceID id ) { return s_particleEmitterRegistry.at( id ); }
	static ParticleEmitterDefinition* EditParticleEmitterDefinition( ResourceID id ) { return s_particleEmitterRegistry.at( id ); }

public:
	void AddSpriteResource( ResourceID id, SpriteResource* newResource ) { s_spriteResourceRegistry.insert( SpriteResourceRegistryPair( id, newResource ) ); }
	void AddSpriteSequence( ResourceID id, AnimatedSpriteSequence* newSequence ) { s_spriteSequenceRegistry.insert( SpriteSequenceRegistryPair( id, newSequence ) ); }
	void AddParticleSystem( ResourceID id, ParticleSystemDefinition* newSystem ) { s_particleSystemRegistry.insert( ParticleSystemDefinitionRegistryPair( id, newSystem ) ); }
	void AddParticleEmitter( ResourceID id, ParticleEmitterDefinition* newEmitter ) { s_particleEmitterRegistry.insert( ParticleEmitterDefinitionRegistryPair( id, newEmitter ) ); }


private:

	static ResourceDatabase* s_theResourceDatabase;
	static SpriteResourceRegistryMap s_spriteResourceRegistry;
	static SpriteSequenceRegistryMap s_spriteSequenceRegistry;
	static ParticleSystemDefinitionRegistryMap s_particleSystemRegistry;
	static ParticleEmitterDefinitionRegistryMap s_particleEmitterRegistry;
};