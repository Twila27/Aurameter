#pragma once


//-----------------------------------------------------------------------------
#include "Engine/EngineCommon.hpp"
#include "Engine/Renderer/DebugRenderCommand.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Matrix4x4.hpp"

#include "Engine/Input/TheInput.hpp"


//-----------------------------------------------------------------------------
#define STATIC


//-----------------------------------------------------------------------------
//Operating System
static const char* g_appName = "Aurameter (SD5 Final) by Benjamin Gibson";


//-----------------------------------------------------------------------------
//Entity System
enum EntityType
{
	ENTITY_TYPE_PLAYER,
	ENTITY_TYPE_ENEMY,
	ENTITY_TYPE_BULLET,
	ENTITY_TYPE_DEFLECTOR,
	NUM_ENTITY_TYPES
};
typedef int EntityID;


//-----------------------------------------------------------------------------
//Naming Keys
static const char KEY_TO_SPEND_AURAMETER = VK_ENTER;
static const char KEY_TO_THROW_DEFLECTOR = VK_SPACE;
static const char KEY_TO_MOVE_LEFT_2D = 'A';
static const char KEY_TO_MOVE_RIGHT_2D = 'D';
static const char KEY_TO_MOVE_UP_2D = 'W';
static const char KEY_TO_MOVE_DOWN_2D = 'S';
static const char KEY_TO_SLOWDOWN_CAMERA = 'Q';
static const char KEY_TO_SPEEDUP_CAMERA = 'E';

static const char KEY_TO_BEGIN_GAME = 'P';
static const char KEY_TO_PAUSE_GAME = 'P';
static const char KEY_TO_RETRY_GAME = 'P';
static const char KEY_TO_RETURN_TO_MENU = 'R';

static const ControllerButtons BUTTON_TO_BEGIN_GAME = ControllerButtons::A_BUTTON;
static const ControllerButtons BUTTON_TO_PAUSE_GAME = ControllerButtons::START_BUTTON;
static const ControllerButtons BUTTON_TO_RETRY_GAME = ControllerButtons::A_BUTTON;
static const ControllerButtons BUTTON_TO_RETURN_TO_MENU = ControllerButtons::B_BUTTON;



//-----------------------------------------------------------------------------
//Camera
static const WorldCoords3D CAMERA3D_DEFAULT_POSITION = WorldCoords3D( -5.f, 0.f, 0.f );
static const WorldCoords2D CAMERA2D_DEFAULT_POSITION = WorldCoords2D::ZERO;


//-----------------------------------------------------------------------------
//Lighting
static const unsigned int LIGHTS_IN_SCENE_MAX = 2;


//-----------------------------------------------------------------------------
//Game
typedef std::string TerrainID;
static const WorldCoords2D PLAYER_START_POSITION = WorldCoords2D( 0.f, -5.f );
static const WorldCoords2D TITLE_CARD_WORLD_POSITION = WorldCoords2D( 0.f, -2.5f );
extern const int BULLET_DAMAGE_AMOUNT;
extern const int MAX_AURAMETER;
extern const int SCORE_FOR_KILLING_ENEMY;
extern const int SCORE_FOR_DEFLECTING_BULLET;
extern const float SECONDS_BETWEEN_WAVES;
extern const float SECONDS_BETWEEN_ARENAS;

enum GameState 
{
	GAME_STATE_MAIN_MENU,
	GAME_STATE_SETUP_GAMEPLAY,
	GAME_STATE_PLAYING,
	GAME_STATE_PAUSED,
	GAME_STATE_GAMEOVER,
	GAME_STATE_CLEANUP_GAMEPLAY,
	NUM_GAME_STATES
};
extern GameState GetGameState();
extern bool SetGameState( GameState newState );
extern const char* GetGameStateName( GameState state );
extern int GetHealthForName( const std::string& name );

//-----------------------------------------------------------------------------
//Rendering
static const RenderLayerID UI_TITLE_LAYER_ID = 1000;
static const RenderLayerID TITLE_CARD_LAYER_ID = 900;
static const RenderLayerID WORLD_TRANSITION_LAYER_ID = 800;
static const RenderLayerID BULLET_FX_LAYER_ID = 600;
static const RenderLayerID DEFLECTOR_LAYER_ID = 100;
static const RenderLayerID MAIN_LAYER_ID = 0;
static const RenderLayerID BACKGROUND_LAYER_ID = -1000;