#pragma once

#include "Engine/Memory/ObjectPool.hpp"
#include "Engine/BuildConfig.hpp"
#include "Engine/Core/EngineEvent.hpp"
#include "Engine/EngineCommon.hpp"
#include <map>


//-----------------------------------------------------------------------------
typedef unsigned long long uint64_t;
struct SampleRecord;
typedef std::pair< float, SampleRecord* > FrametimeOrderedMapPair;
typedef std::multimap< float, SampleRecord* > FrametimeOrderedMap;


//-----------------------------------------------------------------------------
struct ProfilerSample
{
public:
	~ProfilerSample();

	uint64_t initialPerfCount;
	uint64_t elapsedPerfCount;
	const char* tag;
	ProfilerSample* parent;
	ProfilerSample* children;

	//In-place linked list:
	ProfilerSample* prev;
	ProfilerSample* next;

	//For memory tracking:
	int numAllocations;
	int numDeallocations;
	int numTotalBytesAllocated;
	int numTotalBytesFreed;
};


//-----------------------------------------------------------------------------
enum ProfilerReportMode { LIST_VIEW, FLAT_VIEW };


//-----------------------------------------------------------------------------
struct SampleRecord //Separate from ProfilerSample because # samples != # records.
{
	const char* tag;
	int numCalls;
	uint64_t elapsedPerfCount;
	uint64_t selfTimePerfCount; //elapsedPerfCount - total children elapsedPerfCount.
	float totalPercentFrametime; //100 * elapsedPerfCount / totalFrameElapsedPerfCount. 
		//Stored on map key, but needed here for list view since iter aren't passed.

	//For memory:
	int numAllocations;
	int numDeallocations;
	int numTotalBytesAllocated;
	int numTotalBytesFreed;

	//Maintain tree structure for list view:
	SampleRecord* prev;
	SampleRecord* next;
	SampleRecord* parent;
	SampleRecord* children;
};


//-----------------------------------------------------------------------------
class Profiler
{
public:
	static Profiler* Instance();
	Profiler();
	ProfilerReportMode GetReportMode() const { return m_reportingMode; }
	bool GetShouldBeEnabled() const { return m_shouldBeEnabled; }

#if PROFILER_MODE == PROFILER_FULL_FRAME_SAMPLING
	void Startup();
	void Shutdown();

	bool IsProfilerActive() const { return m_currentlyEnabled; }
	void ToggleProfiler() { m_shouldBeEnabled = !m_currentlyEnabled; }
	void SetProfilerPaused( bool isPaused ) { m_shouldBeEnabled = !isPaused; }

	//Manages tree. Factory structure like this is important--we DO NOT want to hit the heap at all, or false positives galore.
	void StartSection( const char* tag ) { StartSample( tag ); }
	void EndSection() { EndSample( m_currentSample ); }

	void StartFrame();
	ProfilerSample* StartSample( const char* tag );
	void EndSample( ProfilerSample* sample );
	void DeleteSample( ProfilerSample* sample );

	ProfilerSample* GetLastFrame() const { return m_previousFrameRootSample; }
	void PrintLastFrame();
	void ToggleReportMode() { m_reportingMode = ( m_reportingMode == LIST_VIEW ) ? FLAT_VIEW : LIST_VIEW; }
	void SetReportingMode( ProfilerReportMode newMode ) { m_reportingMode = newMode; }
	void LogSampleListView( SampleRecord* recursiveRecord, int depth = 0 ); 
	void LogSampleFlatView( const FrametimeOrderedMap& formattedRecords );

	bool MemoryAllocatedEvent_Handler( EngineEvent* ev );
	bool MemoryFreedEvent_Handler( EngineEvent* ev );

#else
	bool IsProfilerActive() {}
	void ToggleProfiler() {}
	void SetProfilerPaused( bool ) {}
	void StartSection( const char* ) {}
	void EndSection() {}
	ProfilerSample* StartFrame() { return nullptr; }
	ProfilerSample* StartSample( const char* ) {}
	void EndSample( ProfilerSample* ) {}
	void DeleteSample( ProfilerSample* ) {}
	ProfilerSample* GetLastFrame() const { return nullptr; }
	void PrintLastFrame() {}
	void ToggleReportMode() {}
	void LogSampleListView( SampleRecord* ) {}
	void LogSampleFlatView( const FrametimeOrderedMap& ) {}
	void Startup() {}
	void Shutdown() {}
#endif


private:
	ProfilerReportMode m_reportingMode;
	static Profiler* m_theProfiler;
	void CreateOrUpdateRecordForSample( ProfilerSample* recursingSample, FrametimeOrderedMap& out_records, int depth = 0 );
	void RecursivelyUpdateMemoryAllocated( ProfilerSample* recursingSample, int numBytesAllocated );
	void RecursivelyUpdateMemoryFreed( ProfilerSample* recursingSample, int numBytesFreed );


	bool m_currentlyEnabled; //Won't switch to below bool until frame completes.
	bool m_shouldBeEnabled; //Triggered immediately by requests to start/stop.
	ProfilerSample* m_currentSample; //Active node we've traversed down to along the current frame tree of samples.
	ProfilerSample* m_currentFrameRootSample; //Root of this frame's tree.
	ProfilerSample* m_previousFrameRootSample; //Root of last frame's tree.
	ObjectPool< ProfilerSample > m_samplesPool;

	uint64_t m_cachedTotalFramePerfCount; //Used for reporting to calculate %frame-time.
;};
