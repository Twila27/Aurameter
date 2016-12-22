#pragma once


#include "Engine/Concurrency/Thread.hpp"
#include "Engine/Concurrency/ThreadSafeQueue.hpp"
#include "Engine/Concurrency/ThreadSafeVector.hpp"


//-----------------------------------------------------------------------------
enum LoggerFilterMode
{
	FILTER_MODE_BLACKLIST,
	FILTER_MODE_WHITELIST,
	NUM_FILTER_MODES
};
#define DEFAULT_FILTER "DefaultTag"


//-----------------------------------------------------------------------------
struct Callstack;
struct Message
{
	const char* tag;
	Callstack* callstack;
	char* buffer;
	unsigned int length;

	Message() : buffer( nullptr ), length( 0 ), tag( DEFAULT_FILTER ) {}
	Message( const char* msg, unsigned int length, const char* tag = DEFAULT_FILTER ) : tag( tag ), length( length ), callstack( nullptr )
	{
		buffer = (char*)malloc( length + 1 ); //To hold the null char on the end.
		strncpy_s( buffer, length + 1, msg, length );
	}
	~Message() { free( buffer ); } //malloc and free to keep memory tracking from getting mangled between threads.
};


//-----------------------------------------------------------------------------
class Logger
{
private:
	static FILE* m_logFile;
	static Thread* m_ioThread; //Dedicated just to this.
	static bool m_isRunning;
	static ThreadSafeQueue<Message*>* m_messageQueue;
	static ThreadSafeVector<const char*>* m_activeFilters;
	static LoggerFilterMode m_currentFilterMode;
	static const int NUM_IGNORED_STACK_FRAMES = 3;

	static void LoggerThreadEntry( void* ); //Main dedicated I/O loop.
	static bool ShouldRun() { return m_isRunning; }
	static void ProcessRemainingMessages();
	static void HandleMessage( const Message* msg );
	static bool IsFilteredOut( const char* tag );
	static void vaPrint( const char* tag, const char* messageFormat, va_list variableArgumentList, bool includeCallstack = false );
	static void EnqueueMessage( const char* messageLiteral, int messageLength, bool includeCallstack = false );
	

public:
	static void Startup();
	static void Shutdown() { m_isRunning = false; m_ioThread->ThreadJoin(); } //Triggers cleanup in ThreadEntry.
	
	static void Flush(); //Calls fflush but also empties message queue.
	static void ListActiveFilters();
	static void ToggleFilterMode();
	static void AddFilter( const char* tag );
	static void RemoveFilter( const char* tag );
	static void RemoveAllFilters();

	static void Printf( const char* messageFormat, ... );
	static void PrintfWithTag( const char* tag, const char* messageFormat, ... );
	static void PrintfWithCallstack( const char* messageFormat, ... );
	static void PrintfWithTagAndCallstack( const char* tag, const char* messageFormat, ... );

	static void RegisterConsoleCommands();
};
