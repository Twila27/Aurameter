#include "Engine/Renderer/FramebufferEffect.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC FboEffectRegistryMap	FramebufferEffect::s_fboEffectRegistry;


//--------------------------------------------------------------------------------------------------------------
STATIC FramebufferEffect* FramebufferEffect::CreateAndAddFboEffectRenderer( const std::string& effectName, std::shared_ptr<Mesh> mesh, Material* material )
{
	TODO( "Just have all such MeshRenderers take a class constant Mesh* fboQuad kept in TheRenderer or built here." );

	MeshRenderer* fboRenderer = new MeshRenderer( mesh, material );
	FramebufferEffect* fboEffect = new FramebufferEffect( fboRenderer );

	s_fboEffectRegistry.insert( FboEffectRegistryPair( effectName, fboEffect ) );

	return fboEffect;
}


//--------------------------------------------------------------------------------------------------------------
FramebufferEffect* FramebufferEffect::GetFboEffect( const std::string& effectName )
{
	return s_fboEffectRegistry.at( effectName );
}


//--------------------------------------------------------------------------------------------------------------
MeshRenderer* FramebufferEffect::GetFboEffectRenderer( unsigned int index )
{
	if ( index >= s_fboEffectRegistry.size() )
		return nullptr;

	unsigned int currentIndex = 0;
	for ( FboEffectRegistryMap::iterator fboIter = s_fboEffectRegistry.begin(); fboIter != s_fboEffectRegistry.end(); ++fboIter )
	{
		if ( currentIndex == index )
			return fboIter->second->m_fboEffectRenderer;
		++currentIndex;
	}

	return nullptr;
}
