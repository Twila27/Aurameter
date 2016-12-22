#include "Game/TheGame.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/PhysicsUtils.hpp"
#include "Engine/Input/TheInput.hpp"
#include "Engine/Audio/TheAudio.hpp"
#include "Engine/Core/Command.hpp"
#include "Engine/Core/TheConsole.hpp"
#include "Engine/Math/Camera2D.hpp"
#include "Engine/Math/Camera3D.hpp"

#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Renderer/SpriteResource.hpp"
#include "Engine/Renderer/SpriteRenderer.hpp"
#include "Engine/Renderer/FramebufferEffect.hpp"

#include "Game/GameCommon.hpp"

#include "Game/Entities/Player.hpp"
#include "Game/Entities/Bullet.hpp"
#include "Game/Entities/Enemy.hpp"
#include "Game/Entities/Deflector.hpp"
#include "Game/Factories/Pattern.hpp"
#include "Game/Factories/EnemyFactory.hpp"
#include "Game/Factories/BulletFactory.hpp"
#include "Game/World.hpp"

#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/TheEventSystem.hpp"
#include "Engine/Renderer/AnimatedSprite.hpp"

#include "Engine/Renderer/Particles/ParticleSystem.hpp"
#include "Engine/Renderer/Particles/ParticleEmitter.hpp"
#include "Engine/Renderer/Particles/ParticleSystemDefinition.hpp"
#include "Engine/Renderer/Particles/ParticleEmitterDefinition.hpp"

#include "Engine/Concurrency/JobUtils.hpp"


//--------------------------------------------------------------------------------------------------------------
TheGame* g_theGame = nullptr;


//--------------------------------------------------------------------------------------------------------------
TheGame::TheGame()
	: m_currentArena( -1 )
	, m_numWaveEnemiesLeft( 0 )
	, m_fadeoutTimer( 0.f )
	, m_FADEOUT_LENGTH_SECONDS( 15.f )
	, m_delayBetweenWaves( SECONDS_BETWEEN_WAVES, "OnWaveTransitionEnd" )
	, m_delayBeforeBackgroundSwap( SECONDS_BETWEEN_ARENAS * .5f, "OnBackgroundHidden" )
	, m_delayBetweenArenas( SECONDS_BETWEEN_ARENAS, "OnArenaTransitionEnd" )
{
	TheEventSystem::Instance()->RegisterEvent< TheGame, &TheGame::SpawnNextWave >( "OnWaveTransitionEnd", this );
}


//--------------------------------------------------------------------------------------------------------------
TheGame::~TheGame()
{
	delete s_playerCamera3D;
}


//--------------------------------------------------------------------------------------------------------------
static void RegisterConsoleCommands()
{
	//AES A1
	g_theConsole->RegisterCommand( "ToggleActiveCameraMode", TheGame::ToggleActiveCameraMode );
	g_theConsole->RegisterCommand( "TogglePolarTranslations2D", TheGame::ToggleActiveCamType2D );
	g_theConsole->RegisterCommand( "TogglePolarTranslations3D", TheGame::ToggleActiveCamType3D );
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::LoadAllXML()
{
	//XML.
	BulletFactory::LoadAllFactories();
	EnemyFactory::LoadAllFactories(); //Depends on Bullets being loaded.
	World::LoadAllArenas(); //Depends on Enemies being loaded.
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::Startup()
{
	RegisterConsoleCommands();
	TheEventSystem::Instance()->RegisterEvent< TheGame, &TheGame::GameEventPatternFired_Handler >( "OnEnemyPatternFired", this );

	s_playerCamera2D->m_usesScrollingLimits = true;
	s_playerCamera2D->m_worldScrollingLimits.mins = WorldCoords2D( -15.f );
	s_playerCamera2D->m_worldScrollingLimits.maxs = WorldCoords2D( 15.f );

	InitSceneRendering();

	InitSpriteRenderer();

	SetupParticleSystems();

	Deflector::LoadAllDeflectors();
	LoadAllXML();
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::Shutdown()
{	
	Deflector::CleanupRegistry();
	SpawnPattern::CleanupRegistry();
	EnemyFactory::CleanupFactories();
	BulletFactory::CleanupFactories();
	World::CleanupRegistry();
	SpriteRenderer::Shutdown();
}


//--------------------------------------------------------------------------------------------------------------
TerrainID TheGame::GetCurrentTerrain() const
{
	return m_arenaCycle[ m_currentArena ]->m_terrain;
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::Update( float deltaSeconds )
{
	UpdateCamera( deltaSeconds );

	bool didUpdate = false;
	switch ( GetGameState() )
	{
		case GAME_STATE_MAIN_MENU: didUpdate = UpdateMainMenu(); break;
		case GAME_STATE_SETUP_GAMEPLAY: didUpdate = true;  UpdateSetupGameplay(); break;
		case GAME_STATE_PLAYING: didUpdate = UpdatePlaying( deltaSeconds ); break;
		case GAME_STATE_PAUSED: didUpdate = UpdatePaused(); break;
		case GAME_STATE_GAMEOVER: didUpdate = UpdateGameOver( deltaSeconds ); break;
		case GAME_STATE_CLEANUP_GAMEPLAY: didUpdate = true;  UpdateCleanupGameplay(); break;
	}

	m_delayBeforeBackgroundSwap.Update( deltaSeconds );
	m_delayBetweenWaves.Update( deltaSeconds );
	m_delayBetweenArenas.Update( deltaSeconds );

	if ( m_newlyAddedEntities.size() > 0 )
	{
		for ( GameEntity* ge : m_newlyAddedEntities )
			m_entities.push_back( ge );

		m_newlyAddedEntities.clear();
	}

	SpriteRenderer::Update( deltaSeconds ); //Culling should be handled by TheRenderer itself, see code review. What about animation updates?
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::UpdateMainMenu()
{
	bool didUpdate = false;

	if ( g_theInput->WasKeyPressedOnce( KEY_TO_BEGIN_GAME ) || g_theInput->WasButtonPressedOnce( BUTTON_TO_BEGIN_GAME, Controllers::CONTROLLER_ONE ) )
	{
		m_fadeoutTimer = m_FADEOUT_LENGTH_SECONDS;
		didUpdate = SetGameState( GAME_STATE_SETUP_GAMEPLAY );
	}
	TODO( "Exit is Escape key for now. What about controller? Need to figure out the isQuitting situation from code review." );

	return didUpdate;
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::UpdatePlaying( float deltaSeconds )
{
	bool didUpdate = false;

	if ( g_theInput->WasKeyPressedOnce( KEY_TO_PAUSE_GAME ) || g_theInput->WasButtonPressedOnce( BUTTON_TO_PAUSE_GAME, Controllers::CONTROLLER_ONE ) )
	{
		didUpdate = SetGameState( GAME_STATE_PAUSED );
	}
	else
	{
		didUpdate = UpdatePlayingEntities( deltaSeconds );
		if ( m_player->IsExpired() )
		{
			SetGameState( GAME_STATE_GAMEOVER );
		}
		else if ( m_numWaveEnemiesLeft <= 0 && m_delayBetweenArenas.IsPaused() ) //Don't want to spawn a wave during an arena change.
		{
			m_delayBetweenWaves.Start();
		}
	}

	return didUpdate;
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::UpdatePaused()
{
	bool didUpdate = false;

	if ( g_theInput->WasKeyPressedOnce( KEY_TO_PAUSE_GAME ) || g_theInput->WasButtonPressedOnce( BUTTON_TO_PAUSE_GAME, Controllers::CONTROLLER_ONE ) )
	{
		didUpdate = SetGameState( GAME_STATE_PLAYING );
	}
	else if ( g_theInput->WasKeyPressedOnce( KEY_TO_RETURN_TO_MENU ) || g_theInput->WasButtonPressedOnce( BUTTON_TO_RETURN_TO_MENU, Controllers::CONTROLLER_ONE ) )
	{
		didUpdate = SetGameState( GAME_STATE_CLEANUP_GAMEPLAY );
	}

	return didUpdate;
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::UpdateGameOver( float deltaSeconds )
{
	bool didUpdate = false;

	m_fadeoutTimer = GetMax( 0.f, m_fadeoutTimer - deltaSeconds );

	UpdatePlayingEntities( deltaSeconds );

	if ( g_theInput->WasKeyPressedOnce( KEY_TO_RETURN_TO_MENU ) || g_theInput->WasButtonPressedOnce( BUTTON_TO_RETURN_TO_MENU, Controllers::CONTROLLER_ONE ) )
	{
		didUpdate = SetGameState( GAME_STATE_CLEANUP_GAMEPLAY );
	}
	if ( g_theInput->WasKeyPressedOnce( KEY_TO_RETRY_GAME ) || g_theInput->WasButtonPressedOnce( BUTTON_TO_RETRY_GAME, Controllers::CONTROLLER_ONE ) )
	{
		DestroyGameplayEntities();
		didUpdate = SetGameState( GAME_STATE_SETUP_GAMEPLAY );
	}

	UNREFERENCED( deltaSeconds );
	return didUpdate;
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::DestroyGameplayEntities()
{
	for ( GameEntity* g : m_entities )
		delete g;
	m_entities.clear();

	for ( World* w : m_arenaCycle )
		delete w;
	m_arenaCycle.clear();

	delete m_player;
	m_player = nullptr;

	m_currentArena = -1;
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::UpdateSetupGameplay()
{
	//Clone off a copy of the world registry in the same sequence.
	World::CloneRegistry( m_arenaCycle );

	//Setup first world.
	AdvanceWorld();

	//Setup player.
	if ( m_player == nullptr )
		m_player = new Player();
	m_player->SetPosition( PLAYER_START_POSITION );
	m_player->Enable();
	GivePlayerDeflector();

	SetGameState( GAME_STATE_PLAYING );
}

//--------------------------------------------------------------------------------------------------------------
void TheGame::UpdateCleanupGameplay()
{
	DestroyGameplayEntities();

	m_delayBeforeBackgroundSwap.Stop();
	m_delayBetweenWaves.Stop();
	m_delayBetweenArenas.Stop();

	SetGameState( GAME_STATE_MAIN_MENU );
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::UpdatePlayingEntities( float deltaSeconds )
{
	bool didUpdate = true;

	UpdatePlayer( deltaSeconds );

	UpdatePlayingEntities_JobApproach( m_entities, deltaSeconds ); //Update everything via job system before applying entity-type-specific logic.

	for ( std::vector<GameEntity*>::iterator entityIter = m_entities.begin(); entityIter != m_entities.end(); )
	{
		GameEntity* currentEntity = *entityIter;

		if ( currentEntity->IsEntityType( ENTITY_TYPE_DEFLECTOR ) )
			UpdateDeflector( currentEntity );

		if ( currentEntity->IsEntityType( ENTITY_TYPE_BULLET ) )
			UpdateBullet( currentEntity );

		if ( currentEntity->IsExpired() )
		{
			if ( currentEntity->IsEntityType( ENTITY_TYPE_ENEMY ) )
			{
				m_player->AddAurameterDelta( ENTITY_TYPE_ENEMY );
				--m_numWaveEnemiesLeft;
			}
			delete currentEntity;
			entityIter = m_entities.erase( entityIter );
		}
		else ++entityIter;
	}

	return didUpdate;
}


//--------------------------------------------------------------------------------------------------------------
static void UpdateGameplayEntities_Job( Job* job )
{
	GameEntity** entities = job->Read<GameEntity**>(); //Parses the data out of the job argument.
	size_t numEntities = job->Read<size_t>();
	float deltaSeconds = job->Read<float>();

	for ( size_t i = 0; i < numEntities; i++ )
	{
		GameEntity* ent = entities[ i ];
		//Probably move this to a method on entity.
		ent->Update( deltaSeconds ); //CRASHES HERE IF ENTITY LIST SIZE > 100! BAD VECTOR PASS?
	}
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::UpdatePlayingEntities_JobApproach( std::vector<GameEntity*>& entities, float deltaSeconds )
{
	size_t splitSize = 100;
	size_t index = 0;
	std::vector<Job*> jobs;

	GameEntity** entityArray = entities.data();
	size_t numEntities = entities.size();

	while ( index < numEntities )
	{
		size_t numEntitiesForJob = GetMin( splitSize, numEntities - index );

		Job* job = JobSystem::Instance()->CreateJob( JOB_CATEGORY_GENERIC, UpdateGameplayEntities_Job );
		job->Write<GameEntity**>( entityArray + index );
		job->Write<size_t>( numEntitiesForJob );
		job->Write<float>( deltaSeconds );
		JobSystem::Instance()->DispatchJob( job );

		jobs.push_back( job );

		index += numEntitiesForJob;
	}

	JobSystem::Instance()->WaitOnJobsForCompletion( jobs );
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::UpdateBullet( GameEntity* currentEntity )
{
	bool canHitPlayer = !currentEntity->HasBeenDeflected() && !m_player->IsExpired();
	AABB2f bulletBounds = currentEntity->GetSprite()->GetVirtualBoundsInWorld();
	AABB2f playerBounds = m_player->GetSprite()->GetVirtualBoundsInWorld();
	if ( canHitPlayer && DoAABBsOverlap( bulletBounds, playerBounds ) )
	{
		m_player->SubtractHealthDelta( BULLET_DAMAGE_AMOUNT );
		int playerHealth = m_player->GetHealth();
		if ( playerHealth <= 0 ) 
		{
			m_player->GetSprite()->Disable();
			ParticleSystem::Play( "Explosion", MAIN_LAYER_ID, m_player->GetPosition() );
		}
		else if ( playerHealth < m_player->GetMaxHealth() * .25f ) //25% health.
		{
			if ( m_player->m_smokeParticleSystem == nullptr ) //Parent under player or make it follow them.
			{
				m_player->m_smokeParticleSystem = ParticleSystem::Create( "Smoke", MAIN_LAYER_ID, m_player->GetPosition() );
			}
		}
		currentEntity->SubtractHealthDelta( currentEntity->GetHealth() );
	}

	bool canHitEnemy = currentEntity->HasBeenDeflected();
	if ( canHitEnemy )
	{
		for ( GameEntity* hitCandidate : m_entities )
		{
			if ( !hitCandidate->IsEntityType( ENTITY_TYPE_ENEMY ) )
				continue;

			if ( DoAABBsOverlap( bulletBounds, hitCandidate->GetSprite()->GetVirtualBoundsInWorld() ) )
			{
				currentEntity->SubtractHealthDelta( currentEntity->GetHealth() );
				hitCandidate->SubtractHealthDelta( BULLET_DAMAGE_AMOUNT );
				ParticleSystem::Play( "Spark", BULLET_FX_LAYER_ID, bulletBounds.GetCenter() ); TODO( "Should be collision point, not bullet center." );
			}
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::UpdateDeflector( GameEntity* currentEntity )
{
	WorldCoords2D deflectorPos = currentEntity->GetPosition();
	AABB2f deflectorBounds = currentEntity->GetSprite()->GetVirtualBoundsInWorld();
	deflectorBounds.maxs += Vector2f( .5f, .5f ); //FUDGE.
	deflectorBounds.mins -= Vector2f( .5f, .5f ); //FUDGE.


	for ( GameEntity* hitCandidate : m_entities ) //Note player is not in this list.
	{
		if ( hitCandidate == currentEntity || hitCandidate->HasBeenDeflected() || !hitCandidate->IsEntityType( ENTITY_TYPE_BULLET ) )
			continue;

		if ( DoAABBsOverlap( deflectorBounds, hitCandidate->GetSprite()->GetVirtualBoundsInWorld() ) )
		{
			m_player->AddAurameterDelta( ENTITY_TYPE_BULLET );
			Deflector* currentDeflector = dynamic_cast<Deflector*>( currentEntity );
			currentDeflector->m_deflectLogic( hitCandidate->GetLinearDynamicsState() );
			hitCandidate->GetSprite()->SetMaterial( Material::CreateOrGetMaterial( "OverrideExample" ) );
			hitCandidate->SetHasBeenDeflected( true );
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::UpdatePlayer( float deltaSeconds )
{
	//Keeping player separate from entity list to always update them first.
	AABB2f playerBounds = m_player->GetSprite()->GetVirtualBoundsInWorld();
	if ( !m_player->IsExpired() )
	{
		m_player->Update( deltaSeconds );

		//Constrain player to world.
		const WorldCoords2D& playerPos = m_player->GetPosition();
		const Vector2f& worldDimsHalved = SpriteRenderer::GetVirtualScreenDimensions() * 0.5f;

		if ( playerPos.x < -worldDimsHalved.x )
			m_player->SetPosition( WorldCoords2D( -worldDimsHalved.x, playerPos.y ) );
		if ( playerPos.y < -worldDimsHalved.y )
			m_player->SetPosition( WorldCoords2D( playerPos.x, -worldDimsHalved.y ) );
		if ( playerPos.x > worldDimsHalved.x )
			m_player->SetPosition( WorldCoords2D( worldDimsHalved.x, playerPos.y ) );
		if ( playerPos.y > worldDimsHalved.y )
			m_player->SetPosition( WorldCoords2D( playerPos.x, worldDimsHalved.y ) );

		if ( m_player->IsGeomancing() )
		{
			GivePlayerDeflector();
		}
	}
}

//--------------------------------------------------------------------------------------------------------------
bool TheGame::GameEventPatternFired_Handler( EngineEvent* eventContext )
{
	std::vector<GameEntity*>& firedBullets = dynamic_cast<GameEventPatternFired*>( eventContext )->m_firedBullets;
	for ( GameEntity* ge : firedBullets )
		m_newlyAddedEntities.push_back( ge );

	return false;
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::SetupParticleSystems()
{
	ParticleSystemDefinition* sparksSystem = ParticleSystem::Register( "Spark" );

	//Add an emitter to it
	ParticleEmitterDefinition* sparksEmitter = sparksSystem->AddEmitter( ParticleEmitterDefinition::Create( "Cross Bullet", false ) );
	sparksEmitter->SetBlendState( PARTICLE_BLEND_STATE_ALPHA_ADDITIVE );
	sparksEmitter->m_initialSpawnCount = 10;
	sparksEmitter->m_secondsPerSpawn = 0.f;
	sparksEmitter->m_initialVelocity = Interval<Vector2f>( Vector2f( -5.5f ), Vector2f( 5.5f ) );
	sparksEmitter->m_lifetimeSeconds = Interval<float>( 1.f, 1.5f );
	sparksEmitter->m_initialScale = Interval<Vector2f>( Vector2f( 2.5f ), Vector2f( 2.7f ) );
	sparksEmitter->m_tint = Rgba( .1f, .8f, .1f, .2f ); //TRANSLUCENT GREEN SPARKS (positive feedback that they died).
	ASSERT_OR_DIE( !sparksEmitter->IsLooping(), nullptr );

	ParticleSystemDefinition* boomSystem = ParticleSystem::Register( "Explosion" );

	//Add an emitter to it
	ParticleEmitterDefinition* boomEmitter = boomSystem->AddEmitter( ParticleEmitterDefinition::Create( "boom_particle", true ) );
	boomEmitter->SetBlendState( PARTICLE_BLEND_STATE_ALPHA_ADDITIVE );
	boomEmitter->m_initialSpawnCount = 100;
	boomEmitter->m_initialVelocity = Interval<Vector2f>( Vector2f( -5.5f ), Vector2f( 5.5f ) );
	boomEmitter->m_secondsPerSpawn = 0.f;
	boomEmitter->m_lifetimeSeconds = Interval<float>( 1.f, 1.5f );
	boomEmitter->m_initialScale = Interval<Vector2f>( Vector2f( .5f ), Vector2f( .7f ) );
	boomEmitter->m_tint = Rgba( .8f, .8f, .1f, .2f ); //TRANSLUCENT YELLOW EXPLOSION.
	ASSERT_OR_DIE( !boomEmitter->IsLooping(), nullptr );

	ParticleSystemDefinition* smokeSystem = ParticleSystem::Register( "Smoke" );

	//Add an emitter to it
	ParticleEmitterDefinition* smokeEmitter = smokeSystem->AddEmitter( ParticleEmitterDefinition::Create( "soft_particle", false ) );
	smokeEmitter->SetBlendState( PARTICLE_BLEND_STATE_ALPHA );
	smokeEmitter->m_initialSpawnCount = 10;
	smokeEmitter->m_secondsPerSpawn = .2f;
	smokeEmitter->m_lifetimeSeconds = Interval<float>( .75f, 1.f );	
	smokeEmitter->m_initialVelocity = Interval<Vector2f>( Vector2f( -1.5f ), Vector2f( 1.5f ) );
	smokeEmitter->m_initialScale = Interval<Vector2f>( Vector2f( .2f ), Vector2f( .4f ) );
	smokeEmitter->m_tint = Rgba( .4f, .4f, .42f ); //GRAY SMOKE.
	ASSERT_OR_DIE( smokeEmitter->IsLooping(), nullptr );
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::InitSpriteRenderer()
{
	SpriteRenderer::LoadAllSpriteResources();

	SpriteRenderer::Startup( s_playerCamera2D, Rgba::DARK_GRAY, true );

	SpriteRenderer::SetDefaultVirtualSize( 15 ); 
	/*
		Suggested to make 1 unit == size of the player's ship sprite.
		HOWEVER, do NOT base virtual or import size on an asset--these should both be decided prior to making assets/game.
		Nuclear Throne example:
			- 240p import size, implying a pixel is 1/240th of the screen (or there are 240 rows of pixels in the window).
			- 15 virtual size, implying a 15x15 grid of virtual units is what the screen can see in total.
			-> Therefore, a single virtual unit takes up 240/15 px == 16 (by 16) px on screen.
			-> Therefore, the average size of a sprite, e.g. the player sprite, should also be 16x16 px.
	*/
	SpriteRenderer::SetImportSize( 240 ); // # vertical lines of resolution.

	SpriteRenderer::UpdateAspectRatio( (float)g_theRenderer->GetScreenWidth(), (float)g_theRenderer->GetScreenHeight() ); //SET THIS at any moment the resolution is changed in the options.

	RegisterSprites();
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::RegisterSprites()
{
	m_player = new Player();

	Sprite* title = Sprite::Create( "Title_UI", WorldCoords2D( 0.f, 5.f ) );
	title->SetLayerID( UI_TITLE_LAYER_ID, "UI" );
	SpriteRenderer::SetLayerVirtualSize( UI_TITLE_LAYER_ID, 23, 23 );
	SpriteRenderer::SetLayerIsScrolling( UI_TITLE_LAYER_ID, false );
	SpriteRenderer::AddLayerEffect( UI_TITLE_LAYER_ID, FramebufferEffect::GetFboEffect( "FboEffect_PostProcessObama" ) );
	title->Enable();

//	Sprite* changingBullet = AnimatedSprite::Create( "Expanding Bullet", WorldCoords2D( -5.f, 0.f ) );
//	changingBullet->Enable();
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::AdvanceWorld()
{
	TheEventSystem* eventSystem = TheEventSystem::Instance();

	if ( m_currentArena >= 0 )
	{
		eventSystem->UnregisterEvent( "OnBackgroundHidden" );
		TheEventSystem::Instance()->RegisterEvent< World, &World::HideBackground >( "OnBackgroundHidden", m_arenaCycle[ m_currentArena ] );
	}

	//For now, just loop when we run out of arenas.
	++m_currentArena;
	m_currentArena = WrapNumberWithinCircularRange( m_currentArena, 0, m_arenaCycle.size() );

	TODO( "Animated Transitions" );
	m_arenaCycle[ m_currentArena ]->m_titleCard->Enable();
	m_arenaCycle[ m_currentArena ]->m_transition->Enable();
	

	eventSystem->UnregisterEvent( "OnArenaTransitionEnd" );
	eventSystem->UnregisterEvent( "OnBackgroundHidden" );


	eventSystem->RegisterEvent< World, &World::FinishTransition >( "OnArenaTransitionEnd", m_arenaCycle[ m_currentArena ] );
	eventSystem->RegisterEvent< World, &World::ShowBackground >( "OnBackgroundHidden", m_arenaCycle[ m_currentArena ] );

	m_delayBetweenArenas.Start();
	m_delayBeforeBackgroundSwap.Start();

	m_numWaveEnemiesLeft = 0;
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::SpawnNextWave( EngineEvent* )
{
	if ( m_currentArena < 0 )
		return false;

	World* currentArena = m_arenaCycle[ m_currentArena ];

	if ( currentArena->m_currentWave >= (int)currentArena->m_waves.size() )
	{
		currentArena->m_currentWave = 0; //For cycling around next time.
		AdvanceWorld();
	}
	else m_numWaveEnemiesLeft = currentArena->SpawnNextWave( m_entities );

	return false;
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::GivePlayerDeflector()
{
	Deflector* newDeflector = m_player->AddDeflector( GetCurrentTerrain() );
	m_entities.push_back( newDeflector );
}