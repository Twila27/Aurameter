#pragma once


//--------------------------------------------------------------------------------------------------------------
void* operator new( size_t numBytes );
void* operator new[] ( size_t numBytesEntireArray );
void operator delete( void* ptr );
void operator delete[] ( void* ptr );


//--------------------------------------------------------------------------------------------------------------
class MemoryAnalytics
{
public:
	static void Startup();
	static void Shutdown();

	static void Update( float deltaSeconds );

	static unsigned int GetCurrentNumAllocations();
	static unsigned int GetAllocationsAtStartup();
	static unsigned int GetCurrentTotalAllocatedBytes();
	static unsigned int GetCurrentHighwaterMark();
	static float GetLastChangeInBytesAllocated();
	static float GetSecondsPerAverage();
	static float GetAverageMemoryChangeRate();

private:
	static unsigned int m_numAllocationsAtStartup;
};