#include "Engine/Memory/Callstack.hpp"

#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include <Windows.h>
#include <DbgHelp.h>

#include "Engine/Memory/CBuffer.hpp"
#include "Engine/EngineCommon.hpp"
#include "Engine/Memory/UntrackedAllocator.hpp"

//--------------------------------------------------------------------------------------------------------------
#define MAX_CALLSTACK_DEPTH 128


//--------------------------------------------------------------------------------------------------------------
typedef BOOL ( __stdcall* sym_initialize_t )( IN HANDLE hProcess, IN PSTR UserSearchPath, IN BOOL fInvadeProcess );
typedef BOOL ( __stdcall* sym_cleanup_t )( IN HANDLE hPROCESS );
typedef BOOL ( __stdcall* sym_from_addr_t )( IN HANDLE hProcess, IN DWORD64 Address, OUT PDWORD64 Displacement, OUT PSYMBOL_INFO Symbol );

typedef BOOL ( __stdcall* sym_get_line_t )( IN HANDLE hProcess, IN DWORD64 dwAddr, OUT PDWORD pdwDisplacement, OUT PIMAGEHLP_LINE64 Symbol );


//--------------------------------------------------------------------------------------------------------------
static HMODULE g_debugHelp;
static HANDLE g_process;
static SYMBOL_INFO* g_symbol;

static CallstackLine g_callstackBuffer[ MAX_CALLSTACK_DEPTH ];

static sym_initialize_t LSymInitialize;
static sym_cleanup_t LSymCleanup;
static sym_from_addr_t LSymFromAddr;
static sym_get_line_t LSymGetLineFromAddr64;


//--------------------------------------------------------------------------------------------------------------
STATIC void Callstack::InitCallstackSystem()
{
	g_debugHelp = LoadLibraryA( "dbghelp.dll" );
	ASSERT_RETURN( g_debugHelp );
	
	LSymInitialize = (sym_initialize_t)GetProcAddress( g_debugHelp, "SymInitialize" );
	LSymCleanup = (sym_cleanup_t)GetProcAddress( g_debugHelp, "SymCleanup" );
	LSymFromAddr = (sym_from_addr_t)GetProcAddress( g_debugHelp, "SymFromAddr" );
	LSymGetLineFromAddr64 = (sym_get_line_t)GetProcAddress( g_debugHelp, "SymGetLineFromAddr64" );

	g_process = GetCurrentProcess();
	LSymInitialize( g_process, NULL, TRUE );

	g_symbol = (SYMBOL_INFO*)malloc( sizeof( SYMBOL_INFO ) + ( MAX_FILENAME_LENGTH * sizeof( char ) ) );
	g_symbol->MaxNameLen = MAX_FILENAME_LENGTH;
	g_symbol->SizeOfStruct = sizeof( SYMBOL_INFO );
}


//--------------------------------------------------------------------------------------------------------------
STATIC void Callstack::DeinitCallstackSystem()
{
	LSymCleanup( g_process );

	free( g_symbol );

	FreeLibrary( g_debugHelp );
	g_debugHelp = NULL;
}


//--------------------------------------------------------------------------------------------------------------
STATIC Callstack* Callstack::FetchAndAllocate( unsigned int stackFramesToSkip )
{
	void* stack[ MAX_CALLSTACK_DEPTH ];
	uint32_t framesTemp = CaptureStackBackTrace( stackFramesToSkip + 1, MAX_CALLSTACK_DEPTH, stack, NULL );

	size_t allocSize = sizeof( Callstack ) + sizeof( void* ) * framesTemp; //Note from below that here framesTemp <=> # frames.
	void* bufferData = malloc( allocSize );

	CBuffer buffer;
	buffer.Initialize( bufferData, allocSize );

	Callstack* cs = buffer.WriteToBuffer<Callstack>();
	buffer.readHeadOffsetFromStart = buffer.writeHeadOffsetFromStart; //We want below Read() to read from AFTER the above WriteTo.
	cs->stackFrames = buffer.ReadFromBuffer<void*>();
	cs->stackFrameCount = framesTemp; //Note pointer trick.
	memcpy( cs->stackFrames, stack, sizeof( void* ) * cs->stackFrameCount );

	return cs;
}


//--------------------------------------------------------------------------------------------------------------
STATIC CallstackLine* Callstack::FetchHumanReadableLines( Callstack* cs )
{
	IMAGEHLP_LINE64 LineInfo;
	DWORD LineDisplacement = 0; //Displacement from the beginning of the line.
	LineInfo.SizeOfStruct = sizeof( IMAGEHLP_LINE64 );

	unsigned int count = cs->stackFrameCount;
	for ( unsigned int i = 0; i < count; ++i )
	{
		CallstackLine* line = &( g_callstackBuffer[ i ] );
		DWORD64 ptr = (DWORD64)( cs->stackFrames[ i ] );
		LSymFromAddr( g_process, ptr, 0, g_symbol );

		strncpy_s( line->functionName, g_symbol->Name, MAX_SYMBOL_NAME_LENGTH );

		BOOL bRet = LSymGetLineFromAddr64(
			GetCurrentProcess(), //Process handle of the current process
			ptr, //Address
			&LineDisplacement, //Displacement stored here by the function
			&LineInfo //File name/line info stored here
		);

		if ( bRet )
		{
			line->line = LineInfo.LineNumber;

			const char* filename = LineInfo.FileName;
			filename += 0; //"Skip to the important bit, so it can be double-clicked in Output."
			strncpy_s( line->filename, filename, 128 );

			line->offset = LineDisplacement;
		}
		else
		{
			line->line = 0;
			line->offset = 0;
			strncpy_s( line->filename, "N/A", 128 );
		}
	}

	return g_callstackBuffer;
}


//--------------------------------------------------------------------------------------------------------------
STATIC void Callstack::FreeCallstack( Callstack* cs )
{
	//Would free(cs->frames) separately, if you allocated it separately.
	free( cs ); //Upside to doing the 1-allocation CBuffer approach, we only need this line.
}


//--------------------------------------------------------------------------------------------------------------
STATIC void Callstack::PrintHumanReadableCallstackToDebugger( Callstack* cs )
{
	DebuggerPrintf( "Top\n" );

	IMAGEHLP_LINE64 LineInfo;
	DWORD LineDisplacement = 0; //Displacement from the beginning of the line.
	LineInfo.SizeOfStruct = sizeof( IMAGEHLP_LINE64 );

	unsigned int count = cs->stackFrameCount;
	for ( unsigned int i = 0; i < count; ++i )
	{
		CallstackLine* line = &( g_callstackBuffer[ i ] );
		DWORD64 ptr = (DWORD64)( cs->stackFrames[ i ] );
		LSymFromAddr( g_process, ptr, 0, g_symbol );

		strncpy_s( line->functionName, g_symbol->Name, MAX_SYMBOL_NAME_LENGTH );
		
		BOOL bRet = LSymGetLineFromAddr64(
			GetCurrentProcess(), //Process handle of the current process
			ptr, //Address
			&LineDisplacement, //Displacement stored here by the function
			&LineInfo //File name/line info stored here
			);

		if ( bRet )
		{
			line->line = LineInfo.LineNumber;

			const char* filename = LineInfo.FileName;
			filename += 0; //"Skip to the important bit, so it can be double-clicked in Output."
			strncpy_s( line->filename, filename, 128 );

			line->offset = LineDisplacement;
		}
		else
		{
			line->line = 0;
			line->offset = 0;
			strncpy_s( line->filename, "N/A", 128 );
		}

		DebuggerPrintf( "\t%s(%d)\n", line->filename, line->line );
	}

	DebuggerPrintf( "Bottom\n\n\n" );
}


//--------------------------------------------------------------------------------------------------------------
void Callstack::PrintHumanReadableCallstackToFile( Callstack* cs, FILE* file )
{
	const char* topStr = "Top\n";
	fwrite( topStr, sizeof( char ), strlen( topStr ), file );

	IMAGEHLP_LINE64 LineInfo;
	DWORD LineDisplacement = 0; //Displacement from the beginning of the line.
	LineInfo.SizeOfStruct = sizeof( IMAGEHLP_LINE64 );

	unsigned int count = cs->stackFrameCount;
	for ( unsigned int i = 0; i < count; ++i )
	{
		CallstackLine* line = &( g_callstackBuffer[ i ] );
		DWORD64 ptr = (DWORD64)( cs->stackFrames[ i ] );
		LSymFromAddr( g_process, ptr, 0, g_symbol );

		strncpy_s( line->functionName, g_symbol->Name, MAX_SYMBOL_NAME_LENGTH );

		BOOL bRet = LSymGetLineFromAddr64(
			GetCurrentProcess(), //Process handle of the current process
			ptr, //Address
			&LineDisplacement, //Displacement stored here by the function
			&LineInfo //File name/line info stored here
			);

		if ( bRet )
		{
			line->line = LineInfo.LineNumber;

			const char* filename = LineInfo.FileName;
			filename += 0; //"Skip to the important bit, so it can be double-clicked in Output."
			strncpy_s( line->filename, filename, 128 );

			line->offset = LineDisplacement;
		}
		else
		{
			line->line = 0;
			line->offset = 0;
			strncpy_s( line->filename, "N/A", 128 );
		}

		char lineBuffer[ 128 + 2 + MAX_SYMBOL_NAME_LENGTH + 128 + 2 ]; //File + Line # + Function Name.
		sprintf_s( lineBuffer, MAX_SYMBOL_NAME_LENGTH + 128, "\t%s(%d) -- %s\n", line->filename, line->line, line->functionName );
		fwrite( lineBuffer, sizeof( char ), strlen( lineBuffer ), file );
	}

	const char* bottomStr = "Bottom\n\n\n";
	fwrite( bottomStr, sizeof( char ), strlen( bottomStr ), file );
}
