#include "Engine/Memory/Memory.hpp"

#include <map>
#include "Engine/Memory/Callstack.hpp"
#include "Engine/Memory/UntrackedAllocator.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/BuildConfig.hpp"
#include "Engine/EngineCommon.hpp"
#include "Engine/Core/Command.hpp"
#include "Engine/Core/TheConsole.hpp"
#include "Engine/Core/TheEventSystem.hpp"
#include "Engine/Core/EngineEvent.hpp"


//--------------------------------------------------------------------------------------------------------------
typedef std::pair< void*, Callstack* > AllocationToCallstackPair;
typedef std::map< void*, Callstack*, std::less<void*>, UntrackedAllocator<AllocationToCallstackPair>  > AllocationToCallstackMap;
static AllocationToCallstackMap* g_callstackRegistry = nullptr;


//--------------------------------------------------------------------------------------------------------------
static bool s_hasTrackerStarted = false;
static unsigned int s_numberOfAllocations = 0;
static unsigned int s_totalAllocatedBytes = 0;
static unsigned int s_maxTotalAllocatedBytes = 0;
STATIC unsigned int MemoryAnalytics::m_numAllocationsAtStartup = 0;


static unsigned int s_numAllocationsSinceLastUpdate = 0;
static int s_changeInBytesAllocated = 0;
static float s_SECONDS_PER_AVERAGE = 1.f; //i.e. Value x will give the average over the last x seconds.
static float s_ONE_OVER_SECONDS_PER_AVERAGE = ( 1.f / s_SECONDS_PER_AVERAGE );
static float s_changeInAllocationOverTime = 0.f;
static float s_changeInAllocationOverAllocations = 0.f;

static const int NUM_IGNORED_STACK_FRAMES = 1;


//--------------------------------------------------------------------------------------------------------------
void* operator new( size_t numBytes )
{
	size_t* ptr = (size_t*)malloc( numBytes + sizeof( size_t ) );
	//DebuggerPrintf( "Alloc %p of %u bytes.\n", ptr, numBytes );
	++s_numberOfAllocations;
	++s_numAllocationsSinceLastUpdate;
	s_totalAllocatedBytes += numBytes;

	if ( s_totalAllocatedBytes > s_maxTotalAllocatedBytes )
		s_maxTotalAllocatedBytes = s_totalAllocatedBytes;

	*ptr = numBytes;

#if MEMORY_DETECTION_MODE >= MEMORY_DETECTION_BASIC
	MemoryAllocatedEvent ev;
	ev.numBytes = numBytes;
	TheEventSystem::Instance()->TriggerEvent( "OnMemoryAllocated", &ev );
#endif

#if MEMORY_DETECTION_MODE == MEMORY_DETECTION_VERBOSE
	if ( s_hasTrackerStarted )
	{
		if ( g_callstackRegistry == nullptr )
		{
			g_callstackRegistry = (AllocationToCallstackMap*)malloc( sizeof( AllocationToCallstackMap ) );
			new( g_callstackRegistry ) AllocationToCallstackMap(); //NOTE: must call dtor explicitly, and then free(), not just delete now.
		}
		g_callstackRegistry->insert( AllocationToCallstackPair( (void*)ptr, Callstack::FetchAndAllocate( NUM_IGNORED_STACK_FRAMES ) ) );
		//Note this occurs before we ++ it back, because we want to track the ENTIRE allocation.
	}
#endif

	ptr++; //Advance back to the actual data allocated.


	return ptr;
}


//--------------------------------------------------------------------------------------------------------------
void* operator new[] ( size_t numBytesEntireArray )
{

	size_t* ptr = (size_t*)malloc( numBytesEntireArray + sizeof( size_t ) );
	//DebuggerPrintf( "Alloc %p of %u bytes.\n", ptr, numBytes );
	++s_numberOfAllocations;
	++s_numAllocationsSinceLastUpdate;
	s_totalAllocatedBytes += numBytesEntireArray;

	*ptr = numBytesEntireArray;

#if MEMORY_DETECTION_MODE >= MEMORY_DETECTION_BASIC
	MemoryAllocatedEvent ev;
	ev.numBytes = numBytesEntireArray;
	TheEventSystem::Instance()->TriggerEvent( "OnMemoryAllocated", &ev );
#endif

#if MEMORY_DETECTION_MODE == MEMORY_DETECTION_VERBOSE
	if ( s_hasTrackerStarted )
	{
		if ( g_callstackRegistry == nullptr )
		{
			g_callstackRegistry = (AllocationToCallstackMap*)malloc( sizeof( AllocationToCallstackMap ) );
			new( g_callstackRegistry ) AllocationToCallstackMap(); //NOTE: must call dtor explicitly, and then free(), not just delete now.
		}
		g_callstackRegistry->insert( AllocationToCallstackPair( (void*)ptr, Callstack::FetchAndAllocate( NUM_IGNORED_STACK_FRAMES ) ) );
		//Note this occurs before we ++ it back, because we want to track the ENTIRE allocation.
	}
#endif

	ptr++; //Advance back to the actual data allocated.
	return ptr;
}


//--------------------------------------------------------------------------------------------------------------
void operator delete( void* ptr )
{
	size_t* ptrSize = (size_t*)ptr;
	--ptrSize;
	size_t numBytes = *ptrSize;

	--s_numberOfAllocations;
	--s_numAllocationsSinceLastUpdate;
	s_totalAllocatedBytes -= numBytes;

	free( ptrSize ); //Free knows how to free the entire malloc, it's not tied to the size_t.

#if MEMORY_DETECTION_MODE >= MEMORY_DETECTION_BASIC
	MemoryFreedEvent ev;
	ev.numBytes = numBytes;
	TheEventSystem::Instance()->TriggerEvent( "OnMemoryFreed", &ev );
#endif

#if MEMORY_DETECTION_MODE == MEMORY_DETECTION_VERBOSE
	if ( s_hasTrackerStarted )
	{
		if ( g_callstackRegistry != nullptr )
		{
			g_callstackRegistry->erase( ptrSize ); //The entire allocation, including packed-in metadata.
		}
	}
#endif
}


//--------------------------------------------------------------------------------------------------------------
void operator delete[] ( void* ptr )
{
	size_t* ptrSize = (size_t*)ptr;
	--ptrSize;
	size_t numBytes = *ptrSize;

	--s_numberOfAllocations;
	--s_numAllocationsSinceLastUpdate;
	s_totalAllocatedBytes -= numBytes;

	free( ptrSize ); //Free knows how to free the entire malloc, it's not tied to the size_t.

#if MEMORY_DETECTION_MODE >= MEMORY_DETECTION_BASIC
	MemoryFreedEvent ev;
	ev.numBytes = numBytes;
	TheEventSystem::Instance()->TriggerEvent( "OnMemoryFreed", &ev );
#endif

#if MEMORY_DETECTION_MODE == MEMORY_DETECTION_VERBOSE
	if ( s_hasTrackerStarted )
	{
		if ( g_callstackRegistry != nullptr )
		{
			g_callstackRegistry->erase( ptrSize ); //The entire allocation, including packed-in metadata.
		}
	}
#endif
}


//--------------------------------------------------------------------------------------------------------------
STATIC void Callstack::PrintCallstacks()
{
	if ( g_callstackRegistry == nullptr )
		return;

	if ( g_callstackRegistry->size() == 0 )
	{
		DebuggerPrintf( "=== Callstack Registry All Clear! :) ===\n" );
		return;
	}

	int pairIndex = 0;
	unsigned int total = g_callstackRegistry->size();
	for ( const AllocationToCallstackPair& callstack : *g_callstackRegistry )
	{
		DebuggerPrintf( "=== Callstack #%d of %u ===\n", pairIndex + 1, total );
		DebuggerPrintf( "\tData Location: %p\n", callstack.first );
		Callstack::PrintHumanReadableCallstackToDebugger( callstack.second );
		++pairIndex;
	}
}


//--------------------------------------------------------------------------------------------------------------
void ToggleMemoryDebugWindow( Command& )
{
	g_showDebugMemoryWindow = !g_showDebugMemoryWindow;
}


//--------------------------------------------------------------------------------------------------------------
void PrintCallstacksCommand( Command& )
{
	Callstack::PrintCallstacks();
}


//--------------------------------------------------------------------------------------------------------------
STATIC void MemoryAnalytics::Startup()
{
	s_hasTrackerStarted = true;

#if MEMORY_DETECTION_MODE >= MEMORY_DETECTION_BASIC

	Callstack::InitCallstackSystem();

	DebuggerPrintf( "\nBASIC MEMORY TRACKING: System found %u allocations prior to entering Main.\n\n", s_numberOfAllocations );
	m_numAllocationsAtStartup = s_numberOfAllocations;

	#if MEMORY_DETECTION_MODE == MEMORY_DETECTION_VERBOSE
	DebuggerPrintf( "VERBOSE MEMORY TRACKING:\n" );
	DebuggerPrintf( "\ts_numberOfAllocations: %u\n", s_numberOfAllocations );
	DebuggerPrintf( "\ts_totalAllocatedBytes: %u\n", s_totalAllocatedBytes );
	DebuggerPrintf( "\ts_maxTotalAllocatedBytes: %u\n", s_maxTotalAllocatedBytes );
	
	g_theConsole->RegisterCommand( "Memory_Flush", PrintCallstacksCommand ); //+2 allocations.
	#endif

	g_theConsole->RegisterCommand( "Memory_Debug", ToggleMemoryDebugWindow ); //+2 allocations.
#endif
}



//--------------------------------------------------------------------------------------------------------------
STATIC void MemoryAnalytics::Shutdown()
{

#if MEMORY_DETECTION_MODE >= MEMORY_DETECTION_BASIC

	if ( s_numberOfAllocations != m_numAllocationsAtStartup )
	{
		DebuggerPrintf( "\n\n/!\\/!\\ %d Memory Leak Report! \
						\nCurrent # Allocations:\t\t%u\
						\nStartup's # Allocations:\t%u\
						\nContinue in Verbose Mode for callstack printouts.\n", 
						s_numberOfAllocations - m_numAllocationsAtStartup,
						s_numberOfAllocations,
						m_numAllocationsAtStartup );
		__debugbreak();

		#if MEMORY_DETECTION_MODE == MEMORY_DETECTION_VERBOSE
		{
			DebuggerPrintf( "Current Total Bytes Allocated: %u\n", s_totalAllocatedBytes );
			DebuggerPrintf( "Highwater Mark for Bytes Allocated: %u\n\n", s_maxTotalAllocatedBytes );

			Callstack::PrintCallstacks();
			__debugbreak();

			g_callstackRegistry->~AllocationToCallstackMap();
			free( g_callstackRegistry );
			g_callstackRegistry = nullptr;
		}
		#endif
	}
	else
	{
		DebuggerPrintf( "\nBASIC MEMORY TRACKING: System on exit found to match the %u allocations prior to entering Main. No leaks! :) \n\n", s_numberOfAllocations );
	}

	Callstack::DeinitCallstackSystem();

#endif
}


//--------------------------------------------------------------------------------------------------------------
static float s_averageReportingTimer = 0.f;
void MemoryAnalytics::Update( float deltaSeconds )
{
	s_averageReportingTimer += deltaSeconds;
	if ( s_averageReportingTimer > s_SECONDS_PER_AVERAGE )
	{
		//Recalculate the average bytes allocated and freed since the last time this section was entered.
		static int previousTotalBytesAllocated = 0;
		s_changeInBytesAllocated = (int)( s_totalAllocatedBytes - previousTotalBytesAllocated );
		s_changeInAllocationOverTime = s_changeInBytesAllocated * s_ONE_OVER_SECONDS_PER_AVERAGE;
	
		s_changeInAllocationOverAllocations = ( s_numAllocationsSinceLastUpdate > 0 ) ? ( s_changeInBytesAllocated / (float)s_numAllocationsSinceLastUpdate ) : 0.f;
		s_numAllocationsSinceLastUpdate = 0;

		previousTotalBytesAllocated = s_totalAllocatedBytes;
		s_averageReportingTimer = 0.f;
	}
}


//--------------------------------------------------------------------------------------------------------------
STATIC unsigned int MemoryAnalytics::GetCurrentNumAllocations()
{
	return s_numberOfAllocations;
}


//--------------------------------------------------------------------------------------------------------------
STATIC unsigned int MemoryAnalytics::GetAllocationsAtStartup()
{
	return MemoryAnalytics::m_numAllocationsAtStartup;
}


//--------------------------------------------------------------------------------------------------------------
STATIC unsigned int MemoryAnalytics::GetCurrentTotalAllocatedBytes()
{
	return s_totalAllocatedBytes;
}


//--------------------------------------------------------------------------------------------------------------
STATIC unsigned int MemoryAnalytics::GetCurrentHighwaterMark()
{
	return s_maxTotalAllocatedBytes;
}


//--------------------------------------------------------------------------------------------------------------
float MemoryAnalytics::GetLastChangeInBytesAllocated()
{
	return s_changeInAllocationOverTime;
}


//--------------------------------------------------------------------------------------------------------------
float MemoryAnalytics::GetSecondsPerAverage()
{
	return s_SECONDS_PER_AVERAGE;
}


//--------------------------------------------------------------------------------------------------------------
float MemoryAnalytics::GetAverageMemoryChangeRate()
{
	return s_changeInAllocationOverAllocations;
}
