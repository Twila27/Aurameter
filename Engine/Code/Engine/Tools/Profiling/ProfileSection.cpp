#include "Engine/Tools/Profiling/ProfileSection.hpp"
#include "Engine/Time/Time.hpp"


#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <time.h>



//--------------------------------------------------------------------------------------------------------------
void ProfileSection::Start()
{
	m_startPerfCount = GetCurrentPerformanceCount();
}


//--------------------------------------------------------------------------------------------------------------
void ProfileSection::End()
{
	m_elapsedPerfCount = GetCurrentPerformanceCount() - m_startPerfCount;
}


//--------------------------------------------------------------------------------------------------------------
double ProfileSection::GetElapsedSeconds() const
{
	return PerformanceCountToSeconds( m_elapsedPerfCount );
}
