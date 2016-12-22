#pragma once


#include <map>
#include <memory>
#include "Engine/Memory/UntrackedAllocator.hpp"


//-----------------------------------------------------------------------------
class Mesh;
class MeshRenderer;
class Material;
class FramebufferEffect;
typedef std::pair< std::string, FramebufferEffect* > FboEffectRegistryPair;
typedef std::map< std::string, FramebufferEffect*, std::less<std::string>, UntrackedAllocator<FboEffectRegistryPair> > FboEffectRegistryMap;


//-----------------------------------------------------------------------------
class FramebufferEffect
{
public:
	FramebufferEffect( MeshRenderer* fboEffectRenderer ) : m_fboEffectRenderer( fboEffectRenderer ) {}
	MeshRenderer* m_fboEffectRenderer;

	static FramebufferEffect* CreateAndAddFboEffectRenderer( const std::string& effectName, std::shared_ptr<Mesh> mesh, Material* material );
	static FramebufferEffect* GetFboEffect( const std::string& effectName );
	static MeshRenderer* GetFboEffectRenderer( unsigned int index );
	static const FboEffectRegistryMap* GetFboEffectRendererRegistry() { return &s_fboEffectRegistry; }


private:
	static FboEffectRegistryMap s_fboEffectRegistry;
};
