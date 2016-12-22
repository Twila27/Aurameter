#pragma once
#include <vector>
#include "Engine/EngineCommon.hpp"


//-----------------------------------------------------------------------------
class Sprite;
class ParticleSystem;
class FramebufferEffect;


//-----------------------------------------------------------------------------
struct RenderLayer
{
public:
	RenderLayer( const RenderLayerID& id, const std::string& name = "", bool enabled = true )
		: m_layerID( id )
		, m_name( name )
		, m_enabled( enabled )
		, m_virtualSize( NO_CUSTOM_VIRTUAL_SIZE )
	{
	}

	void AddParticleSystem( ParticleSystem* ps ) { m_particleSystems.push_back( ps ); }
	void AddSprite( Sprite* sprite ) { m_sprites.push_back( sprite ); }
	void RemoveSprite( Sprite* sprite ) 
	{ 
		for ( unsigned int index = 0; index < m_sprites.size(); index++ )
		{
			if ( m_sprites[ index ] == sprite )
			{
				m_sprites.erase( m_sprites.begin() + index );
				return;
			}
		}
	}
	Vector2f GetLayerVirtualSize() const { return m_virtualSize; }
	void UpdateVirtualSize( float unitXY );
	void UpdateVirtualSize( float unitX, float unitY );
	bool IsScrolling() const { return m_isScrolling; }
	void SetIsScrolling( bool newVal ) { m_isScrolling = newVal; }
	void IgnoreScrolling() { m_isScrolling = false; }
	void AllowScrolling() { m_isScrolling = true; }
	void AddFboEffect( FramebufferEffect* fboEffect );


public:
	std::vector<ParticleSystem*> m_particleSystems; //Note that particle systems render last in layer over everything.
	std::vector<FramebufferEffect*> m_effects;
	std::vector<Sprite*> m_sprites;
	std::string m_name;
	RenderLayerID m_layerID;
	bool m_enabled;
	static const Vector2f NO_CUSTOM_VIRTUAL_SIZE;


private:
	//Each layer can have its own virtual size, but not bounding volume (since I want scrolling limits on Camera2D), only whether or not to ignore scrolling altogether.
	Vector2f m_virtualSize; //NOT a unit length as I had before, but the total size of the layer in virtual units (used to scale sprite size).
	bool m_isScrolling; //When true, all sprites inside this receive uView matrix := identity.
};

//Background layers could go negative from a default of 0.
//Going by intervals: 100 as enemy layer, 200 as player layer, 300 as bullet layer, 400 as foreground, -100 as background, -200 as secondary background, and +1000 as UI layer. Lets you add layers in between without shifting all else.
//Recommends named constants over an enum to not lock it down on the engine side.
