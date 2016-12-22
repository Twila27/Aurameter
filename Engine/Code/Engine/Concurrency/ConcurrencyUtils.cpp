#include "Engine/Concurrency/ConcurrencyUtils.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


//--------------------------------------------------------------------------------------------------------------
extern unsigned int SystemGetCoreCount()
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo( &sysInfo );

	return (unsigned int)sysInfo.dwNumberOfProcessors;
}
