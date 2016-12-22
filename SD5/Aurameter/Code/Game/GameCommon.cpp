#include "Game/GameCommon.hpp"
#include "Engine/Core/TheConsole.hpp"
#include "Engine/Renderer/SpriteRenderer.hpp"


//--------------------------------------------------------------------------------------------------------------
const int BULLET_DAMAGE_AMOUNT = 1;
const int MAX_AURAMETER = 100;
const int SCORE_FOR_KILLING_ENEMY = 25;
const int SCORE_FOR_DEFLECTING_BULLET = 10;
const float SECONDS_BETWEEN_WAVES = 1.f;
const float SECONDS_BETWEEN_ARENAS = 5.f;


//--------------------------------------------------------------------------------------------------------------
static GameState currentState = GAME_STATE_MAIN_MENU;
static GameState previousState;

GameState GetGameState() { return currentState; }
GameState GetPreviousGameState() { return previousState; }
const char* GetGameStateName( GameState state )
{
	switch ( state )
	{
		case GAME_STATE_MAIN_MENU: return "MAIN_MENU";
		case GAME_STATE_SETUP_GAMEPLAY: return "SETUP_GAMEPLAY";
		case GAME_STATE_PLAYING: return "PLAYING";
		case GAME_STATE_PAUSED: return "PAUSED";
		case GAME_STATE_GAMEOVER: return "GAMEOVER";
		case GAME_STATE_CLEANUP_GAMEPLAY: return "CLEANUP_GAMEPLAY";
		case NUM_GAME_STATES: return "NUM_GAME_STATES";
		default: Logger::Printf( "GetStateName Default Branch Unexpectedly Hit!" ); return "Not a State!";
	}
}
bool SetGameState( GameState newState )
{
	bool didChange = false; //When something can fail, add a bool retval for success.

	GameState oldState = GetGameState();
	if ( oldState != newState ) //notice no m_state.
	{
		std::string result = Stringf( "SetState Change: %d => %d!\n", oldState, newState );
		if ( g_inDebugMode )
			g_theConsole->Printf( result.c_str() );
		else
			DebuggerPrintf( result.c_str() );
		oldState = currentState;
		currentState = newState;
		didChange = true;
	}
	else ERROR_AND_DIE( Stringf( "SetState Trying to Change To Same State: %s!", GetGameStateName( newState ) ) );

	if ( newState == GAME_STATE_PLAYING )
		SpriteRenderer::DisableLayer( UI_TITLE_LAYER_ID );
	else
		SpriteRenderer::EnableLayer( UI_TITLE_LAYER_ID );

	return didChange;
}


//--------------------------------------------------------------------------------------------------------------
int GetHealthForName( const std::string& name )
{
	if ( name == "Player" )
		return 5;
	else 
		return 1;
}
