#pragma once
#pragma warning( disable : 4091 ) // Removing warning about typedef on an unnamed enum in DbgHelp.h

#include "Engine/Memory/ByteUtils.hpp"
#include <cstdio>


//--------------------------------------------------------------------------------------------------------------
#define MAX_FILENAME_LENGTH 1024
#define MAX_SYMBOL_NAME_LENGTH 1024
struct CallstackLine
{
	char filename[ MAX_FILENAME_LENGTH ];
	char functionName[ MAX_SYMBOL_NAME_LENGTH ];
	uint32_t line;
	uint32_t offset;
};


//--------------------------------------------------------------------------------------------------------------
struct Callstack
{
	void** stackFrames;
	unsigned int stackFrameCount;

	static void InitCallstackSystem();
	static void DeinitCallstackSystem();

	static Callstack* FetchAndAllocate( unsigned int stackFramesToSkip );
	static CallstackLine* FetchHumanReadableLines( Callstack* cs );
	static void FreeCallstack( Callstack* cs );

	static void PrintCallstacks();
	static void PrintHumanReadableCallstackToDebugger( Callstack* cs ); //Same as FetchLines but prints directly to Output.
	static void PrintHumanReadableCallstackToFile( Callstack* cs, FILE* file );
};
