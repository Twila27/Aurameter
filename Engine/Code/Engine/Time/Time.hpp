#pragma once


//-----------------------------------------------------------------------------------------------
typedef unsigned long long uint64_t;


//-----------------------------------------------------------------------------------------------
extern void SeedWindowsRNG();
extern double GetCurrentTimeSeconds();
extern float CalcDeltaSeconds();
extern uint64_t GetCurrentPerformanceCount();
extern double PerformanceCountToSeconds( uint64_t numOperations );