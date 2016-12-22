#pragma once


#include "Game/GameCommon.hpp"
#include "Engine/Time/Stopwatch.hpp"


//-----------------------------------------------------------------------------
class TheGame;
class Camera3D;
class Camera2D;
class Command;
class Sprite;
class Player;
class GameEntity;
class EngineEvent;
struct World;
struct Job;


//-----------------------------------------------------------------------------
extern TheGame* g_theGame;


//-----------------------------------------------------------------------------
enum CameraMode
{
	CAMERA_MODE_2D,
	CAMERA_MODE_3D
};


//-----------------------------------------------------------------------------
class TheGame
{
public:

	TheGame();
	~TheGame();

	void Startup();
	void Shutdown();
	
	bool IsQuitting() const { return m_isQuitting; }
	void Update( float deltaSeconds );


	bool UpdateMainMenu();
	void UpdateSetupGameplay();
	bool UpdatePlaying( float deltaSeconds );
	bool UpdatePlayingEntities( float deltaSeconds );
	void UpdatePlayingEntities_JobApproach( std::vector<GameEntity*>& entities, float deltaSeconds );
	void UpdateBullet( GameEntity* currentEntity );
	void UpdateDeflector( GameEntity* currentEntity );
	void UpdatePlayer( float deltaSeconds );

	bool UpdatePaused();
	bool UpdateGameOver( float deltaSeconds );
	void UpdateCleanupGameplay();
	void DestroyGameplayEntities();

	bool GameEventPatternFired_Handler( EngineEvent* eventContext );

	void Render3D();
	bool Render2D();
	void RenderDebug3D();
	void RenderDebug2D();

	bool RenderMainMenu2D();
	bool RenderPlaying2D();
	bool RenderPaused2D();
	bool RenderGameOver2D();

	const Camera3D* GetActiveCamera3D() const;
	const Camera2D* GetActiveCamera2D() const;
	static void ToggleActiveCamType3D( Command& );
	static void ToggleActiveCamType2D( Command& );
	static void ToggleActiveCameraMode( Command& );
	
	TerrainID GetCurrentTerrain() const;

private:

	static CameraMode s_activeCameraMode;
	bool m_isQuitting;

	void SetupParticleSystems();

	void InitSpriteRenderer();
	void RegisterSprites();
	void LoadAllXML();

	void InitSceneRendering();
	void CreateShaderPrograms();
	void CreateUniformValues();
	void CreateMeshRenderers();
	void CreateFBOs();
	void CreateSceneLights();

	void RenderLeftDebugText2D();
	void RenderRightDebugText2D();

	void UpdateCamera( float deltaSeconds );
	void UpdateCameraCartesian2D( float deltaSeconds );
	void UpdateCameraPolar2D( float deltaSeconds );
	void UpdateFromKeyboard2D( float deltaSeconds );
	void UpdateFromMouse2D();

	void GivePlayerDeflector();
	bool SpawnNextWave( EngineEvent* eventContext );
	void AdvanceWorld();

	std::vector<GameEntity*> m_entities;
	std::vector<GameEntity*> m_newlyAddedEntities; //Else enemies could add bullets to m_entities midway through a loop m_entities!

	static Camera3D* s_playerCamera3D;
	static Camera2D* s_playerCamera2D;
	Player* m_player;

	Stopwatch m_delayBetweenWaves;
	Stopwatch m_delayBeforeBackgroundSwap;
	Stopwatch m_delayBetweenArenas;
	std::vector< World* > m_arenaCycle;
	int m_currentArena;
	int m_numWaveEnemiesLeft;

	float m_fadeoutTimer;
	const float m_FADEOUT_LENGTH_SECONDS;
};
