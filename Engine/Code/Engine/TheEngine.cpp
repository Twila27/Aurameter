#include "Engine/TheEngine.hpp"

//Major Subsystems
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/TheInput.hpp"
#include "Engine/Audio/TheAudio.hpp"
#include "Engine/Core/TheConsole.hpp"
#include "Engine/Renderer/DebugRenderCommand.hpp"
#include "Game/TheGame.hpp"

//Major Utils
#include "Engine/Time/Time.hpp"
#include "Engine/Tools/FBXUtils.hpp"

//SD5
#include "Engine/Memory/Memory.hpp"
#include "Engine/Core/Logger.hpp"
#include "Engine/Tools/Profiling/Profiler.hpp"


//--------------------------------------------------------------------------------------------------------------
TheEngine* g_theEngine = nullptr;


//--------------------------------------------------------------------------------------------------------------
static void RegisterConsoleCommands()
{
	//AES A1
	g_theConsole->RegisterCommand( "FBXList", FBXList );

	//AES A2
	g_theConsole->RegisterCommand( "FBXLoad", FBXLoad );

	//AES A3
	g_theConsole->RegisterCommand( "MeshSaveLastMeshBuilderMade", MeshSaveLastMeshBuilderMade );
	g_theConsole->RegisterCommand( "MeshLoadFromFile", MeshLoadFromFile );

	//AES A4
	g_theConsole->RegisterCommand( "SkeletonSaveLastSkeletonMade", SkeletonSaveLastSkeletonMade );
	g_theConsole->RegisterCommand( "SkeletonLoadFromFile", SkeletonLoadFromFile );
	g_theConsole->RegisterCommand( "DebugRenderClearCommands", DebugRenderClearCommands );
	TODO( "Make a separate 'add' console command for each of the DebugRenderCommands!" );

	//AES A5
	g_theConsole->RegisterCommand( "AnimationLoadFromFile", AnimationLoadFromFile );
	g_theConsole->RegisterCommand( "AnimationSaveLastAnimationMade", AnimationSaveLastAnimationMade );

	//SD5 A2
	Logger::RegisterConsoleCommands();
}


//--------------------------------------------------------------------------------------------------------------
void TheEngine::Startup( double screenWidth, double screenHeight )
{
	//-----------------------------------------------------------------------------
	//	Allocation/Constructor Calls
	//-----------------------------------------------------------------------------

	//Make sure Renderer ctor comes first so that default texture gets ID of 1. Args configure FBO dimensions.
	g_theRenderer = new TheRenderer( screenWidth, screenHeight );
	g_theDebugRenderCommands = new std::list< DebugRenderCommand* >();

	g_theAudio = new AudioSystem(); //Example usage:
	// [static] SoundID musicID = g_theAudio->CreateOrGetSound( "Data/Audio/Yume Nikki mega mix (SD).mp3" );
	// [g_bgMusicChannel =] g_theAudio->PlaySound( musicID );
		//This is declared as AudioChannelHandle g_bgMusicChannel; necessary to track for things like turning on looping.

	g_theGame = new TheGame();

	g_theInput = new TheInput();
	Vector2i screenCenter = Vector2i( (int)( screenWidth / 2.0 ), (int)( screenHeight / 2.0 ) );
	g_theInput->SetCursorSnapToPos( screenCenter );
	g_theInput->OnGainedFocus();
	g_theInput->HideCursor();

	g_theConsole = new TheConsole(screenWidth/2., screenHeight/2., screenWidth, screenHeight);
	//g_theConsole = new TheConsole( 0.0, 30.0, screenWidth, screenHeight );
	RegisterConsoleCommands();

	//-----------------------------------------------------------------------------
	//	Startup/Initialization Calls
	//-----------------------------------------------------------------------------

	SeedWindowsRNG();

	g_theRenderer->PreGameStartup();
	g_theGame->Startup();
	g_theRenderer->PostGameStartup();
}


//--------------------------------------------------------------------------------------------------------------
void TheEngine::RunFrame()
{
	this->Update( CalcDeltaSeconds() );

#ifdef PLATFORM_RIFT_CV1
	g_theRenderer->UpdateEyePoses();
	if (g_theRenderer->IsRiftVisible())
	{
		for (int eye = 0; eye < 2; eye++)
		{
			g_theRenderer->UpdateRiftForEye( eye );
			this->Render();
			g_theRenderer->CommitFrameToRift( eye );
		}
	}
	g_theRenderer->SubmitFrameToRift();
#else
	this->Render();
#endif
}


//--------------------------------------------------------------------------------------------------------------
void TheEngine::Update( float deltaSeconds )
{
	ProfilerSample* sample = Profiler::Instance()->StartSample( "TheEngine::Update" );

	MemoryAnalytics::Update( deltaSeconds );

	g_theAudio->Update();
	//	if ( g_theInput->WasKeyPressedOnce( 'X' ) ) g_theAudio->StopChannel( g_bgMusicChannel );

	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_DEBUG_INFO ) ) 
		g_inDebugMode = !g_inDebugMode;

	g_theConsole->Update( deltaSeconds ); //Delta +='d into caret's alpha.

	g_theRenderer->Update( deltaSeconds, g_theGame->GetActiveCamera3D() );
		//Update uniforms for shader timers, scene MVP, and lights.

	TODO( "Explore passing in 0 to freeze, or other values to rewind, slow, etc." );
	g_theGame->Update( deltaSeconds );

	UpdateDebugCommands( deltaSeconds );

	Profiler::Instance()->EndSample( sample );
}


//--------------------------------------------------------------------------------------------------------------
void TheEngine::RenderDebug3D()
{
	if ( g_theRenderer->IsShowingAxes() )
		AddDebugRenderCommand( new DebugRenderCommandBasis( Vector3f::ZERO, 1000.f, true, 0.f, DEPTH_TEST_DUAL, 10.f ) );

	g_theGame->RenderDebug3D();
}


//--------------------------------------------------------------------------------------------------------------
void TheEngine::RenderDebug2D()
{
	g_theRenderer->SetBlendFunc( 0x0302, 0x0303 );

	g_theRenderer->EnableBackfaceCulling( false ); //Console needs it.
	g_theRenderer->EnableDepthTesting( false ); //Drawing this on top of everything else.

	g_theGame->RenderDebug2D();

	RenderLeftDebugText2D();
	RenderRightDebugText2D();
}


//--------------------------------------------------------------------------------------------------------------
void TheEngine::RenderDebugMemoryWindow()
{
	TODO( "Subfunction if anything else non-memory gets added!" );

	float screenHeight = (float)g_theRenderer->GetScreenHeight();
	const Vector2f TOP_LEFT = Vector2f( 100.f, screenHeight - 50.f ); //Note +y is up, +x is right.

#if MEMORY_DETECTION_MODE >= MEMORY_DETECTION_BASIC

	static Rgba bgColor = Rgba::MAGENTA * Rgba::DARK_GRAY;
	bgColor.alphaOpacity = 172;
	static Rgba fgColor = Rgba::MAGENTA * Rgba::GRAY;
	fgColor.alphaOpacity = 255;

	static const unsigned int NUM_DEBUG_STRINGS = 6;
	/*static*/ std::string memoryDebugInfo[ NUM_DEBUG_STRINGS ] =
	{
		Stringf( "# Allocations: %u", MemoryAnalytics::GetCurrentNumAllocations() - MemoryAnalytics::GetAllocationsAtStartup() ),
		Stringf( "# Allocations + %u @ Startup: %u", MemoryAnalytics::GetAllocationsAtStartup(), MemoryAnalytics::GetCurrentNumAllocations() ),
		Stringf( "# Total Bytes Allocated: %u", MemoryAnalytics::GetCurrentTotalAllocatedBytes() ),
		Stringf( "Max # Total Bytes Allocated: %u", MemoryAnalytics::GetCurrentHighwaterMark() ),
		Stringf( "Change in Bytes Since %.2fs Ago: %f", MemoryAnalytics::GetSecondsPerAverage(), MemoryAnalytics::GetLastChangeInBytesAllocated() ),
		Stringf( "Average Allocation Size Since %.2fs Ago: %f", MemoryAnalytics::GetSecondsPerAverage(), MemoryAnalytics::GetAverageMemoryChangeRate() )
	};

	static float stringPixelWidths[ NUM_DEBUG_STRINGS ];
	for ( int strIndex = 0; strIndex < NUM_DEBUG_STRINGS; strIndex++ )
	{
		const std::string& currentStr = memoryDebugInfo[ strIndex ];
		stringPixelWidths[ strIndex ] = g_theRenderer->CalcTextPxWidthUpToIndex( currentStr, currentStr.size() );
	}

	float maxWidth = GetMax( stringPixelWidths, NUM_DEBUG_STRINGS );

	Vector2f offsetFromTopLeft = Vector2f( 5.f, -5.f );
	static const float spaceBetweenStrings = 50.f;
	AABB2f bounds = AABB2f( TOP_LEFT - offsetFromTopLeft, TOP_LEFT + Vector2f( maxWidth, -spaceBetweenStrings * NUM_DEBUG_STRINGS ) + offsetFromTopLeft );
	g_theRenderer->DrawShadedAABB( VertexGroupingRule::AS_TRIANGLES, bounds, bgColor, bgColor, fgColor, fgColor );

	for ( int strIndex = 0; strIndex < NUM_DEBUG_STRINGS; strIndex++ )
		g_theRenderer->DrawTextProportional2D( TOP_LEFT - ( Vector2f::UNIT_Y * ( strIndex * spaceBetweenStrings ) ), memoryDebugInfo[ strIndex ] );

#endif
}


//--------------------------------------------------------------------------------------------------------------
void TheEngine::RenderLeftDebugText2D()
{
}


//--------------------------------------------------------------------------------------------------------------
void TheEngine::RenderRightDebugText2D()
{
}


//--------------------------------------------------------------------------------------------------------------
void TheEngine::Render()
{
	ProfilerSample* sample = Profiler::Instance()->StartSample( "TheEngine::Render" );

	g_theRenderer->PreRenderStep();

	//-----------------------------------------------------------------------------
	//Has to stick its own render2d and render3d around TheGame's, so we can't just call to TheGame::Render() here.
	
	g_theRenderer->SetupView3D( g_theGame->GetActiveCamera3D() );
	g_theGame->Render3D();
	if ( g_inDebugMode )
		this->RenderDebug3D();
	RenderThenExpireDebugCommands3D(); //Want it running even if not in debug mode, to cover other sources adding commands.

	g_theRenderer->SetupView2D( g_theGame->GetActiveCamera2D() );
	g_theGame->Render2D();
	if ( g_showDebugMemoryWindow )
		this->RenderDebugMemoryWindow();
	if ( g_inDebugMode )
		this->RenderDebug2D();

	//-----------------------------------------------------------------------------
	g_theRenderer->PostRenderStep();

	g_theConsole->Render();

	Profiler::Instance()->EndSample( sample );

	//Main_Win32 should call TheApp's FlipAndPresent() next.
}


//--------------------------------------------------------------------------------------------------------------
void TheEngine::Shutdown()
{
	//Any other subsystems that have their own Shutdown() equivalent call here.
	ClearDebugCommands();
	g_theGame->Shutdown();
	g_theRenderer->Shutdown();
	g_theConsole->CleanupEntries();

	//-----------------------------------------------------------------------------
	delete g_theGame;
	delete g_theAudio;
	delete g_theRenderer;
	delete g_theDebugRenderCommands;
	delete g_theInput;
	delete g_theConsole;

	//-----------------------------------------------------------------------------
	g_theGame = nullptr;
	g_theAudio = nullptr;
	g_theInput = nullptr;
	g_theRenderer = nullptr;
	g_theDebugRenderCommands = nullptr;
	g_theConsole = nullptr;
}


//--------------------------------------------------------------------------------------------------------------
bool TheEngine::IsQuitting()
{
	return g_theGame->IsQuitting();
}
