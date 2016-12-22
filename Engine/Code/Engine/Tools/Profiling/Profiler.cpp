#include "Engine/Tools/Profiling/Profiler.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/Core/Logger.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/Core/TheConsole.hpp"
#include "Engine/Core/InPlaceLinkedList.hpp"
#include "Engine/Memory/Memory.hpp"
#include "Engine/Core/TheEventSystem.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC Profiler* s_theProfiler = nullptr;


//--------------------------------------------------------------------------------------------------------------
Profiler::Profiler()
	: m_currentlyEnabled( false )
	, m_shouldBeEnabled( false )
	, m_previousFrameRootSample( nullptr )
	, m_currentFrameRootSample( nullptr )
	, m_currentSample( nullptr )
	, m_reportingMode( LIST_VIEW )
{
#if PROFILER_MODE == PROFILER_FULL_FRAME_SAMPLING
	const int MAX_SAMPLES = 10000;
	m_samplesPool.Init( MAX_SAMPLES );
#endif
}


//--------------------------------------------------------------------------------------------------------------
STATIC Profiler* Profiler::Instance()
{
	if ( s_theProfiler == nullptr )
		s_theProfiler = new Profiler();

	return s_theProfiler;
}


#if PROFILER_MODE == PROFILER_FULL_FRAME_SAMPLING
//--------------------------------------------------------------------------------------------------------------
static void PauseProfiler( Command& ) 
{ 
	Profiler::Instance()->SetProfilerPaused( true ); 
	g_theConsole->Printf( "Profiler paused." );
}
static void UnpauseProfiler( Command& ) 
{ 
	Profiler::Instance()->SetProfilerPaused( false ); 
	g_theConsole->Printf( "Profiler unpaused." );
}
static void ToggleProfiler( Command& ) 
{ 
	Profiler::Instance()->ToggleProfiler();
	g_theConsole->Printf( "Profiler toggled from %s.", ( Profiler::Instance()->GetShouldBeEnabled() ) ? "off to on" : "on to off" );
}
static void PrintLastProfiledFrame( Command& ) 
{ 
	Profiler::Instance()->PrintLastFrame();
	g_theConsole->Printf( "Print completed." );
}
static void ProfilerChangeReportView( Command& ) 
{ 
	Profiler::Instance()->ToggleReportMode();
	g_theConsole->Printf( "New Report Mode: %s", ( Profiler::Instance()->GetReportMode() == LIST_VIEW ) ? "List View" : "Flat View" );
}


//--------------------------------------------------------------------------------------------------------------
static void RegisterConsoleCommands()
{
	g_theConsole->RegisterCommand( "ProfilerPause", PauseProfiler );
	g_theConsole->RegisterCommand( "ProfilerUnpause", UnpauseProfiler );
	g_theConsole->RegisterCommand( "ProfilerTogglePaused", ToggleProfiler );
	g_theConsole->RegisterCommand( "ProfilerPrintLastFrame", PrintLastProfiledFrame );
	g_theConsole->RegisterCommand( "ProfilerChangeReportView", ProfilerChangeReportView );
}


//--------------------------------------------------------------------------------------------------------------
void Profiler::Startup()
{
	m_shouldBeEnabled = true;
	RegisterConsoleCommands();

#if MEMORY_DETECTION_MODE > MEMORY_DETECTION_NONE
	TheEventSystem::Instance()->RegisterEvent< Profiler, &Profiler::MemoryAllocatedEvent_Handler >( "OnMemoryAllocated", this );
	TheEventSystem::Instance()->RegisterEvent< Profiler, &Profiler::MemoryFreedEvent_Handler >( "OnMemoryFreed", this );
#endif
}


//--------------------------------------------------------------------------------------------------------------
void Profiler::Shutdown()
{
	m_shouldBeEnabled = false;
	
	//Since this is called from outside the main game loop though, the "frame" will be over.
	this->DeleteSample( m_previousFrameRootSample );
	m_previousFrameRootSample = nullptr;
	this->DeleteSample( m_currentFrameRootSample );
	m_currentFrameRootSample = nullptr;
	this->DeleteSample( m_currentSample );
	m_currentSample = nullptr;

#if MEMORY_DETECTION_MODE > MEMORY_DETECTION_NONE
	TheEventSystem::Instance()->UnregisterSubscriber< Profiler, &Profiler::MemoryAllocatedEvent_Handler >( "OnMemoryAllocated", this );
	TheEventSystem::Instance()->UnregisterSubscriber< Profiler, &Profiler::MemoryFreedEvent_Handler >( "OnMemoryFreed", this );
#endif

	delete s_theProfiler;
	s_theProfiler = nullptr;
}


//--------------------------------------------------------------------------------------------------------------
void Profiler::StartFrame()
{
	//Takes the previous frame's sample, toss it, take prev := curr,
	//and then set g_currentSample and g_currentSample to a new initial sample.

	//FRAME END.
	if ( m_currentlyEnabled )
	{
		ASSERT_OR_DIE( m_currentFrameRootSample == m_currentSample, nullptr ); //i.e. that we have all frames popped off now, we have no stack or tree corruption.

		EndSection(); //A pop in the tree structure, such that g_currentFrame should be null now.

		this->DeleteSample( m_previousFrameRootSample ); //Assuming the dtor null checks and that each node calls delete on its children.
			//Moved after Pop() so it won't get timed.

		m_previousFrameRootSample = m_currentFrameRootSample;
	}

	//Now respond to requests to enable/disable.
	m_currentlyEnabled = m_shouldBeEnabled;

	if ( m_currentlyEnabled )
	{
		static const char* ROOT_SAMPLE_NAME = "Main";

		this->StartSample( ROOT_SAMPLE_NAME );

		m_currentFrameRootSample = m_currentSample; //g_currentFrame is the top of the frame stack-tree, g_currentSample is where we've traversed to so far.	
	}
}


//--------------------------------------------------------------------------------------------------------------
ProfilerSample* Profiler::StartSample( const char* tag )
{
	if ( !m_currentlyEnabled )
		return nullptr;

	ProfilerSample* sample = m_samplesPool.Allocate(); //Custom allocator, so it can potentially "run out of allocators".
	sample->tag = tag;
	sample->parent = m_currentSample;
	if ( m_currentSample != nullptr ) //i.e. Skipped for the root's case.
		Append( m_currentSample->children, sample ); //In-place linked list at work here.
	m_currentSample = sample; //Dropping down in tree depth occurs here.

	//Doing this prior to starting the timer since they hit the heap several times, but will be reflected in any parents' timers.

	sample->initialPerfCount = GetCurrentPerformanceCount();

	TODO( "Better way to express the init value being stored w/o adding another one-use variable to ProfilerSample struct?" );
	sample->numAllocations = 0;
	sample->numDeallocations = 0;
	sample->numTotalBytesAllocated = 0;
	sample->numTotalBytesFreed = 0;

	return sample;
}


//--------------------------------------------------------------------------------------------------------------
void Profiler::EndSample( ProfilerSample* sample )
{
	if ( !m_currentlyEnabled )
		return;

	//CHANGE THIS TO BE 0-arity and just pop based on m_currentSample and its parent ptr. 
	//Implied to be this way by the "assert sample exists", which we also need:
	ASSERT_RETURN( m_currentSample );

	sample->elapsedPerfCount = GetCurrentPerformanceCount() - sample->initialPerfCount;
	m_currentSample = sample->parent;
}


//--------------------------------------------------------------------------------------------------------------
void Profiler::DeleteSample( ProfilerSample* sample )
{
	m_samplesPool.Delete( sample );
}


//--------------------------------------------------------------------------------------------------------------
SampleRecord* g_currentRecord; //For tree setup of records for list view.
void Profiler::CreateOrUpdateRecordForSample( ProfilerSample* recursingSample, FrametimeOrderedMap& out_records, int depth /*= 0*/ )
{
	if ( recursingSample == nullptr )
		return;

	if ( depth == 0 )
		m_cachedTotalFramePerfCount = recursingSample->elapsedPerfCount;

	//Tabulate self-time and frame-time.
	uint64_t totalChildrenPerfCount = 0;
	for ( ProfilerSample* childIter = recursingSample->children; childIter != nullptr; childIter = ( childIter->next == recursingSample->children ) ? nullptr : childIter->next )
		totalChildrenPerfCount += childIter->elapsedPerfCount;
	uint64_t selfTimePerfCount = recursingSample->elapsedPerfCount - totalChildrenPerfCount;

	float percentFrametime = 100.f * recursingSample->elapsedPerfCount / m_cachedTotalFramePerfCount;

	//Because the multimap is handling sorting by frame-time for us, we take on the task of grouping duplicate-tag records.
	//But because the key-value is frame-time, we can't use .count.
	FrametimeOrderedMap::iterator recordIter;
	for ( recordIter = out_records.begin(); recordIter != out_records.end(); ++recordIter )
	{
		if ( strcmp( recordIter->second->tag, recursingSample->tag ) == 0 )
			break;
	}

	//Update record:
	if ( recordIter == out_records.end() ) //Not a duplicate.
	{
		SampleRecord* newRecord = new SampleRecord();
		newRecord->tag = recursingSample->tag;
		newRecord->elapsedPerfCount = recursingSample->elapsedPerfCount;
		newRecord->numCalls = 1;
		newRecord->selfTimePerfCount = selfTimePerfCount;
		newRecord->totalPercentFrametime = percentFrametime;
		newRecord->numAllocations = recursingSample->numAllocations;
		newRecord->numTotalBytesAllocated = recursingSample->numTotalBytesAllocated; //0 if memory detection mode == none.
		newRecord->numDeallocations = recursingSample->numDeallocations;
		newRecord->numTotalBytesFreed = recursingSample->numTotalBytesFreed; //0 if memory detection mode == none.
		out_records.insert( FrametimeOrderedMapPair( percentFrametime, newRecord ) );

		newRecord->parent = g_currentRecord;
		if ( g_currentRecord != nullptr ) //i.e. Skipped for root case.
			Append( g_currentRecord->children, newRecord ); //In-place linked list at work here.
		g_currentRecord = newRecord; //Dropping down in tree depth occurs here.
	}
	else //Duplicate. Have to remove and insert, since map keys are const.
	{
		SampleRecord* repeatedRecord = recordIter->second;
		repeatedRecord->elapsedPerfCount += recursingSample->elapsedPerfCount;
		repeatedRecord->numCalls += 1;
		repeatedRecord->selfTimePerfCount += selfTimePerfCount;
		repeatedRecord->totalPercentFrametime += percentFrametime;
		repeatedRecord->numAllocations += recursingSample->numAllocations;
		repeatedRecord->numTotalBytesAllocated += recursingSample->numTotalBytesAllocated;
		repeatedRecord->numDeallocations += recursingSample->numDeallocations;
		repeatedRecord->numTotalBytesFreed += recursingSample->numTotalBytesFreed;
		out_records.insert( FrametimeOrderedMapPair( repeatedRecord->totalPercentFrametime, repeatedRecord ) );
		out_records.erase( recordIter );
	}

	//Recursion:
	for ( ProfilerSample* childIter = recursingSample->children; childIter != nullptr; childIter = ( childIter->next == recursingSample->children ) ? nullptr : childIter->next )
		CreateOrUpdateRecordForSample( childIter, out_records, depth + 1 );

	g_currentRecord = g_currentRecord->parent;
}


//--------------------------------------------------------------------------------------------------------------
void Profiler::PrintLastFrame()
{
	ProfilerSample* frameRoot = GetLastFrame();

	FrametimeOrderedMap framePercentageSortedMap;
	CreateOrUpdateRecordForSample( frameRoot, framePercentageSortedMap );

	TODO( "Ifdef on memory-mode whether to print the memory information." );
	switch ( m_reportingMode ) 
	{
		case LIST_VIEW: //We send the last element which has highest %frame-time, i.e. 100%, i.e. the root main frame's record.
			LogSampleListView( framePercentageSortedMap.rbegin()->second );
			break;
		case FLAT_VIEW: 
			LogSampleFlatView( framePercentageSortedMap );
			break;
	}


}


//--------------------------------------------------------------------------------------------------------------
void Profiler::LogSampleListView( SampleRecord* recursiveRecord, int depth /*= 0 */ )
{
	//Doesn't matter if it takes long, since this is after profiling has completed.
	if ( recursiveRecord == nullptr )
		return;

	if ( depth == 0 ) //Top of the printout.
	{
		Logger::PrintfWithTag( "Profiler", "%-5s\t %-30s\t %-6s\t %-21s\t %-21s\t %-11s\t %-17s\t %-17s\t",
							   "DEPTH",
							   "TAG",
							   "#CALLS",
							   "TIME (PerfCount, ms)",
							   "SELF-TIME",
							   "%FRAME-TIME",
							   "#ALLOCS (#Bytes)",
							   "#FREES  (#Bytes)" );
	}

	TODO( "Ifdef on memory-mode whether to print the memory information." );
	Logger::PrintfWithTag( "Profiler", "%-5d\t %-30s\t %-6d\t %-7llu (%.6fms)\t %-7llu (%.6fms)\t %-7.2f%%\t\t %-7d (%-5db)\t %-7d (%-5db)\t", 
							depth,
							recursiveRecord->tag,
							recursiveRecord->numCalls,
							recursiveRecord->elapsedPerfCount, PerformanceCountToSeconds( recursiveRecord->elapsedPerfCount ),
							recursiveRecord->selfTimePerfCount, PerformanceCountToSeconds( recursiveRecord->selfTimePerfCount ),
							recursiveRecord->totalPercentFrametime, 
							recursiveRecord->numAllocations, recursiveRecord->numTotalBytesAllocated,
							recursiveRecord->numDeallocations, recursiveRecord->numTotalBytesFreed );

	SampleRecord* childIter;
	for ( childIter = recursiveRecord->children; childIter != nullptr; childIter = ( childIter->next == recursiveRecord->children ) ? nullptr : childIter->next )
		LogSampleListView( childIter, depth + 1 );
}


//--------------------------------------------------------------------------------------------------------------
void Profiler::LogSampleFlatView( const FrametimeOrderedMap& formattedRecords )
{
	Logger::PrintfWithTag( "Profiler", "%-30s\t %-6s\t %-21s\t %-21s\t %-11s\t %-17s\t %-17s\t",
						   "TAG",
						   "#CALLS",
						   "TIME (PerfCount, ms)",
						   "SELF-TIME",
						   "%FRAME-TIME",
						   "#ALLOCS (#Bytes)",
						   "#FREES  (#Bytes)" );

	//Reverse iter: because default map ordering will sort 0% to 100% frame-time, which we want to reverse.
	TODO( "Ifdef on memory-mode whether to print the memory information." );
	for ( FrametimeOrderedMap::const_reverse_iterator recordIter = formattedRecords.rbegin(); recordIter != formattedRecords.rend(); ++recordIter )
	{
		SampleRecord* currentRecord = recordIter->second;
		Logger::PrintfWithTag( "Profiler", "%-30s\t %-6d\t %-7llu (%.6fms)\t %-7llu (%.6fms)\t %-7.2f%%\t\t %-7d (%-5db)\t %-7d (%-5db)\t",
							   currentRecord->tag,
							   currentRecord->numCalls,
							   currentRecord->elapsedPerfCount, PerformanceCountToSeconds( currentRecord->elapsedPerfCount ),
							   currentRecord->selfTimePerfCount, PerformanceCountToSeconds( currentRecord->selfTimePerfCount ),
							   currentRecord->totalPercentFrametime,
							   currentRecord->numAllocations, currentRecord->numTotalBytesAllocated,
							   currentRecord->numDeallocations, currentRecord->numTotalBytesFreed );
	}
}
#endif


//--------------------------------------------------------------------------------------------------------------
ProfilerSample::~ProfilerSample()
{
	if ( children == nullptr )
		return;

	ProfilerSample* childIter;
	for ( childIter = children; childIter != nullptr; childIter = ( childIter->next == children ) ? nullptr : childIter->next )
	{
		Profiler::Instance()->DeleteSample( childIter );
	}
}


//--------------------------------------------------------------------------------------------------------------
void Profiler::RecursivelyUpdateMemoryAllocated( ProfilerSample* recursingSample, int numBytesAllocated )
{
	if ( recursingSample == nullptr )
		return;

	++recursingSample->numAllocations;
	recursingSample->numTotalBytesAllocated += numBytesAllocated;

	for ( ProfilerSample* childIter = recursingSample->children; childIter != nullptr; childIter = ( childIter->next == recursingSample->children ) ? nullptr : childIter->next )
		RecursivelyUpdateMemoryAllocated( childIter, numBytesAllocated );
}


//--------------------------------------------------------------------------------------------------------------
bool Profiler::MemoryAllocatedEvent_Handler( EngineEvent* ev )
{
	int numBytesAllocated = dynamic_cast<MemoryAllocatedEvent*>(ev)->numBytes; 

	//Walk the tree of current samples, updating memory amounts.
	RecursivelyUpdateMemoryAllocated( m_currentFrameRootSample, numBytesAllocated );

	return false; //Do not remove from subscribers.
}


//--------------------------------------------------------------------------------------------------------------
void Profiler::RecursivelyUpdateMemoryFreed( ProfilerSample* recursingSample, int numBytesFreed )
{
	if ( recursingSample == nullptr )
		return;

	++recursingSample->numDeallocations;
	recursingSample->numTotalBytesFreed += numBytesFreed;

	for ( ProfilerSample* childIter = recursingSample->children; childIter != nullptr; childIter = ( childIter->next == recursingSample->children ) ? nullptr : childIter->next )
		RecursivelyUpdateMemoryFreed( childIter, numBytesFreed );
}


//--------------------------------------------------------------------------------------------------------------
bool Profiler::MemoryFreedEvent_Handler( EngineEvent* ev )
{
	int numBytesFreed = dynamic_cast<MemoryFreedEvent*>( ev )->numBytes;

	//Walk the tree of current samples, updating memory amounts.
	RecursivelyUpdateMemoryFreed( m_currentFrameRootSample, numBytesFreed );

	return false; //Do not remove from subscribers.
}
