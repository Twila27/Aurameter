#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Engine/Memory/Memory.hpp"
#include "Engine/Core/Logger.hpp"
#include "Engine/Tools/Profiling/Profiler.hpp"
#include "Engine/Tools/Profiling/ProfileLogSection.hpp"
#include "Engine/Concurrency/JobUtils.hpp"

#include "Game/TheApp.hpp"
#include "Engine/TheEngine.hpp"


//--------------------------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int )
{
	UNREFERENCED_PARAMETER( commandLineString );

	Logger::Startup();
	MemoryAnalytics::Startup();
	Profiler::Instance()->Startup();
	JobSystem::Instance()->Startup( -4 ); //As many as I need, minus 4.

	g_theApp = new TheApp();

	g_theApp->Startup( applicationInstanceHandle );

	while ( !g_theApp->IsQuitting() )
	{
		g_theApp->HandleInput();
		Profiler::Instance()->StartFrame();
		g_theEngine->RunFrame();
		g_theApp->FlipAndPresent();
	}

	g_theApp->Shutdown();

	delete g_theApp;
	g_theApp = nullptr;

	JobSystem::Instance()->Shutdown();
	Profiler::Instance()->Shutdown();
	MemoryAnalytics::Shutdown();
	Logger::Shutdown();

	return 0;
}