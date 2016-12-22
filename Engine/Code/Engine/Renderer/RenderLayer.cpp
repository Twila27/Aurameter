#include "Engine/Renderer/RenderLayer.hpp"
#include "Engine/Renderer/FramebufferEffect.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC const Vector2f RenderLayer::NO_CUSTOM_VIRTUAL_SIZE = Vector2f::ZERO;


//--------------------------------------------------------------------------------------------------------------
void RenderLayer::UpdateVirtualSize( float unitXY )
{
	UpdateVirtualSize( unitXY, unitXY );
}


//--------------------------------------------------------------------------------------------------------------
void RenderLayer::UpdateVirtualSize( float unitX, float unitY )
{
	m_virtualSize = Vector2f( unitX, unitY );
}


//--------------------------------------------------------------------------------------------------------------
void RenderLayer::AddFboEffect( FramebufferEffect* fboEffect )
{
	m_effects.push_back( fboEffect );
}
