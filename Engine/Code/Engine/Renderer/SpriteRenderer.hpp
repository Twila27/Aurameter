#pragma once
#include "Engine/Renderer/RenderLayer.hpp"
#include "Engine/Renderer/RenderState.hpp"
#include "Engine/Memory/UntrackedAllocator.hpp"
#include <map>
#include <memory>
#include "Engine/EngineCommon.hpp"

//--------------------------------------------------------------------------------------------------------------
/* While TheRenderer wraps around OpenGL's API, this more conventional renderer provides:
	- Sprites
		- Dimensions
		- Transform (rotation becomes angle in 2D)
		- Pivot
		- Diffuse Texture
		- Material
		- Sorting Layer ID
	- Timeline-Animation Sprites (NOT YET)
	- Sorting Layers (a la Painter's Algorithm, organized so more positive is more toward foreground)
	- Text (NOT YET THROUGH THIS SYSTEM)
	- Extra Todo:
		- 2D Skeletons (i.e. AES in 2D to get parented sprites, confer Spine middleware)
		- Sockets
		- Parallax

	i.e. It does not handle cases that require limited field of view, like our SD4 roguelike did.
		- Could do shooters.
		- Could do platformers.
*/
//--------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------
class FramebufferEffect;
class FrameBuffer;
class Material;
class Mesh;
class MeshRenderer;
class Command;
class Camera2D;
struct Rgba;
typedef std::pair<RenderLayerID, RenderLayer*> SpriteLayerRegistryPair;
typedef std::map<RenderLayerID, RenderLayer*, std::less<RenderLayerID>, UntrackedAllocator<SpriteLayerRegistryPair> > SpriteLayerRegistryMap;


//-----------------------------------------------------------------------------
class SpriteRenderer
{
public:

	static void Startup( const Camera2D* activeCamera, const Rgba& clearColor, bool shouldHide3D );
	static void LoadAllSpriteResources();
	static void ReloadAllSprites( Command& );
	static void ReloadAllSprites();
	static void SaveAllSprites( Command& arg );
	static void Shutdown();

	static void ToggleSpriteParenting( Command& ) { s_isParentingEnabled = !s_isParentingEnabled; }
	static bool IsParentingEnabled() { return s_isParentingEnabled; }
	static float GetAspectRatio() { return s_aspectRatio; }
	static float GetImportSize() { return s_defaultImportSize; }
	static Vector2f GetDefaultVirtualSize() { return s_defaultVirtualSize; } //Default as in layers without custom virtual sizes use this one.
	static Vector2f GetVirtualScreenWidth() { return s_virtualScreenSize.x; }
	static Vector2f GetVirtualScreenHeight() { return s_virtualScreenSize.y; }
	static Vector2f GetVirtualScreenDimensions() { return s_virtualScreenSize; }
	static unsigned int GetNumSpritesCulled() { return s_numSpritesCulled; }
	static unsigned int GetNumLiveParticles();

	static void SetImportSize( float newSize ) { s_defaultImportSize = newSize; }
	static void SetDefaultVirtualSize( float unitXY ) { s_defaultVirtualSize = Vector2f( unitXY ); }
	static void SetDefaultVirtualSize( float unitX, float unitY ) { s_defaultVirtualSize = Vector2f( unitX, unitY ); } //Use vwidth == vheight to not stretch.
	static void UpdateAspectRatio( float windowWidthPx, float windowHeightPx );
	static void ToggleShowing3D( Command& ) { s_shouldHide3D = !s_shouldHide3D; }
	static void ToggleShowing2D( Command& ) { s_shouldHide2D = !s_shouldHide2D; }
	static void ToggleCulling( Command& ) { s_shouldCull = !s_shouldCull; }

	static void PrintLayers( Command& );
	static void CreateOrRenameLayer( Command& args );
	static void DisableLayer( Command& arg );
	static void EnableLayer( Command& arg );
	static void ToggleLayer( Command& arg );
	static void DisableLayer( RenderLayerID layerID ) { s_spriteLayers[ layerID ]->m_enabled = false; }
	static void EnableLayer( RenderLayerID layerID ) { s_spriteLayers[ layerID ]->m_enabled = true; }
	static void ToggleLayer( RenderLayerID layerID ) { s_spriteLayers[ layerID ]->m_enabled = !s_spriteLayers[ layerID ]->m_enabled; }
	static void SetLayerVirtualSize( RenderLayerID layerID, float lengthXY ) { SetLayerVirtualSize( layerID, lengthXY, lengthXY ); }
	static void SetLayerVirtualSize( RenderLayerID layerID, float lengthX, float lengthY );
	static void SetLayerIsScrolling( RenderLayerID layerID, bool newVal ) { s_spriteLayers.at( layerID )->SetIsScrolling( newVal ); }

	static void ResizeSprite( Sprite* sprite );

	static void Update( float deltaSeconds );
	static void RenderFrame();

	static void DrawLayer( RenderLayer* layer );

	static void Register( Sprite* newSprite, const char* newLayerName = nullptr );
	static void Unregister( Sprite* sprite );
	static RenderLayer* CreateOrGetLayer( RenderLayerID layerID, const char* newLayerName = nullptr );

	static void CopySpriteIntoMesh( Sprite* sprite );

	static void AddLayerEffect( RenderLayerID layerID, FramebufferEffect* fboMaterial );

	static const RenderState s_defaultSpriteRenderState;
	static Material* s_defaultSpriteMaterial;
	static std::shared_ptr<Mesh> s_spriteMesh; //Recopied onto per registered sprite quad.
	static MeshRenderer* s_spriteMeshRenderer;


private:
	static void RenderSprite( Sprite* sprite, bool isLayerScrolling );
	static SpriteLayerRegistryMap s_spriteLayers; //Small enough to have by value.
	static FrameBuffer* s_currentRenderTarget; //What we're currently rendering to. The composite of all of them.
	static FrameBuffer* s_effectRenderTarget; //Secondary, for the effect.
	static Rgba s_clearColor;
	static bool s_shouldHide2D;
	static bool s_shouldHide3D;
	static bool s_shouldCull;
	static bool s_isParentingEnabled;
	static unsigned int s_numSpritesCulled;

	static float s_aspectRatio;
	static float s_defaultImportSize; //Specifies a pixel is 1/importSize-th of the screen.
		//Set to the biggest possible rect of a sprite as authored in DCC tools.
		//e.g. a spritesheet of 16x16 sprites would have 16p.
	static Vector2f s_virtualScreenSize; //This is what we set as our orthographic projection's dimensions.
	static Vector2f s_defaultVirtualSize; //"Default", as in layers without custom virtual sizes use this one.
		//NOT a unit length as I had before, but full # virtual units the screen takes up. 15 is common in sprite games.
		//i.e. Describes the largest square we can center in the middle of the screen when in virtual units.
		//Indirectly controlling how zoomed in on our sprites we will end up.
		//Usually specified in terms of 1 unit == 1 player character sprite, 
			//e.g. a 16x16 ship would have virtual size 16.
			//e.g. Ori and the Blind Forest might likely be 16 Ori's high.
		//Conceptually, this specifies how big the quads/tiles of the screen's map become,
		//Or when thought of in reverse the # of rows of tiles we can have for the screen's map.
			//e.g. Being more pixelated, Nuclear Throne likely authors its sprites at a max import size 240x240 (240p).
		//When virtualSize == importSize, you get the pixels stretching, as in games that don't do the virtual size logic.

	static const Camera2D* s_activeCamera; //Set by the game.
};
