#include "Engine/Core/Logger.hpp"
#include "Engine/BuildConfig.hpp"

#include "Engine/Core/TheConsole.hpp"
#include "Engine/Core/Command.hpp"
#include "Engine/EngineCommon.hpp"
#include "Engine/FileUtils/FileUtils.hpp"
#include "Engine/Concurrency/CriticalSection.hpp"
#include "Engine/Memory/Callstack.hpp"

#include <stdarg.h>

#ifdef LOGGER_OUTPUT_TO_VSDEBUGGER
	#ifdef _WIN32
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#endif


//--------------------------------------------------------------------------------------------------------------
STATIC bool Logger::m_isRunning = false;
STATIC Thread* Logger::m_ioThread = nullptr;
STATIC FILE* Logger::m_logFile = nullptr;
STATIC ThreadSafeQueue<Message*>* Logger::m_messageQueue = nullptr;
STATIC ThreadSafeVector<const char*>* Logger::m_activeFilters = nullptr;
STATIC LoggerFilterMode Logger::m_currentFilterMode = FILTER_MODE_BLACKLIST;


//--------------------------------------------------------------------------------------------------------------
#pragma region Console Commands
static void LoggerPrintf( Command& args )
{
	std::string out = args.GetArgsString();
	if ( out != "" )
	{
		Logger::Printf( out.c_str() );
	}
	else
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: LoggerPrintf <Message>" );
	}
}


//--------------------------------------------------------------------------------------------------------------
static void LoggerPrintfWithTag( Command& args )
{
	std::string argsString = args.GetArgsString();
	size_t firstSpaceIndex = argsString.find_first_of( ' ' );
	std::string outTag = argsString.substr( 0U, firstSpaceIndex );
	std::string outMsg = argsString.substr( firstSpaceIndex + 1, argsString.size() );

	if ( ( outTag != "" ) && ( outMsg != "" ) )
	{
		Logger::PrintfWithTag( outTag.c_str(), outMsg.c_str() );
	}
	else
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: LoggerPrintfWithTag <Tag> <Message>" );
	}
}


//--------------------------------------------------------------------------------------------------------------
static void LoggerPrintfWithCallstack( Command& args )
{
	std::string out = args.GetArgsString();
	if ( out != "" )
	{
		Logger::PrintfWithCallstack( out.c_str() );
	}
	else
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: LoggerPrintfWithCallstack <Message>" );
	}
}


//--------------------------------------------------------------------------------------------------------------
static void LoggerPrintfWithTagAndCallstack( Command& args )
{
	std::string argsString = args.GetArgsString();
	size_t firstSpaceIndex = argsString.find_first_of( ' ' );
	std::string outTag = argsString.substr( 0U, firstSpaceIndex );
	std::string outMsg = argsString.substr( firstSpaceIndex + 1, argsString.size() );

	if ( ( outTag != "" ) && ( outMsg != "" ) )
	{
		Logger::PrintfWithTagAndCallstack( outTag.c_str(), outMsg.c_str() );
	}
	else
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: LoggerPrintfWithCallstackAndTag <Tag> <Message>" );
	}
}


//--------------------------------------------------------------------------------------------------------------
static void LoggerToggleFilterMode( Command& )
{
	Logger::ToggleFilterMode();
}


//-----------------------------------------------------------------------------
static void LoggerPrintTestThreadEntry( void* args )
{
	Logger::Printf( "LoggerTestPrint: Thread #%d", *(int*)args );
	Logger::Flush();
}


//--------------------------------------------------------------------------------------------------------------
static void LoggerPrintTest( Command& )
{
	int a( 0 ), b( 1 ), c( 2 ), d( 3 ), e( 4 ), f( 5 ), g( 6 ), h( 7 );

	Thread threadsForTesting[] =
	{
		Thread( LoggerPrintTestThreadEntry, &a ),
		Thread( LoggerPrintTestThreadEntry, &b ),
		Thread( LoggerPrintTestThreadEntry, &c ),
		Thread( LoggerPrintTestThreadEntry, &d ),
		Thread( LoggerPrintTestThreadEntry, &e ),
		Thread( LoggerPrintTestThreadEntry, &f ),
		Thread( LoggerPrintTestThreadEntry, &g ),
		Thread( LoggerPrintTestThreadEntry, &h )
	};

	for ( Thread& t : threadsForTesting )
		t.ThreadDetach();
}


//--------------------------------------------------------------------------------------------------------------
static void LoggerAddFilter( Command& args )
{
	std::string out;
	bool doesArgExist = args.GetNextString( &out, nullptr );
	if ( doesArgExist && ( out != "" ) )
	{
		Logger::AddFilter( out.c_str() );
	}
	else
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: LoggerAddFilter <Message>" );
	}
}


//--------------------------------------------------------------------------------------------------------------
static void LoggerRemoveFilter( Command& args )
{
	std::string out;
	bool doesArgExist = args.GetNextString( &out, nullptr );
	if ( doesArgExist && ( out != "" ) )
	{
		Logger::RemoveFilter( out.c_str() );
	}
	else
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: LoggerRemoveFilter <Message>" );
	}
}


//--------------------------------------------------------------------------------------------------------------
static void LoggerRemoveAllFilters( Command& )
{
	Logger::RemoveAllFilters();
}


//--------------------------------------------------------------------------------------------------------------
static void LoggerListActiveFilters( Command& )
{
	Logger::ListActiveFilters();
}
#pragma endregion


//--------------------------------------------------------------------------------------------------------------
void Logger::RegisterConsoleCommands()
{
	g_theConsole->RegisterCommand( "LoggerPrintf", LoggerPrintf );
	g_theConsole->RegisterCommand( "LoggerPrintfWithTag", LoggerPrintfWithTag );
	g_theConsole->RegisterCommand( "LoggerToggleFilterMode", LoggerToggleFilterMode );
	g_theConsole->RegisterCommand( "LoggerPrintTest", LoggerPrintTest );

	g_theConsole->RegisterCommand( "LoggerAddFilter", LoggerAddFilter );
	g_theConsole->RegisterCommand( "LoggerRemoveFilter", LoggerRemoveFilter );
	g_theConsole->RegisterCommand( "LoggerRemoveAllFilters", LoggerRemoveAllFilters );
	g_theConsole->RegisterCommand( "LoggerListActiveFilters", LoggerListActiveFilters );

	g_theConsole->RegisterCommand( "LoggerPrintfWithCallstack", LoggerPrintfWithCallstack );
	g_theConsole->RegisterCommand( "LoggerPrintfWithCallstackAndTag", LoggerPrintfWithTagAndCallstack );
}


//--------------------------------------------------------------------------------------------------------------
void Logger::ListActiveFilters()
{
	unsigned int numFilters = m_activeFilters->Size();
	if ( numFilters == 0 )
	{
		g_theConsole->Printf( "No filters active in current %s.", ( m_currentFilterMode == FILTER_MODE_BLACKLIST ) ? "blacklist" : "whitelist" );
		return;
	}
	for ( unsigned int index = 0; index < numFilters; index++ )
	{
		const char* out;
		m_activeFilters->At( index, &out );
		g_theConsole->Printf( out );
	}
}


//--------------------------------------------------------------------------------------------------------------
void Logger::HandleMessage( const Message* msg )
{
	fwrite( &msg->buffer[ 0 ], sizeof( char ), msg->length, m_logFile );

	if ( msg->callstack != nullptr )
		Callstack::PrintHumanReadableCallstackToFile( msg->callstack, m_logFile );

#ifdef LOGGER_OUTPUT_TO_VSDEBUGGER
	if ( IsDebuggerAvailable() )
	{
		OutputDebugStringA( msg->buffer );

		if ( msg->callstack != nullptr )
			Callstack::PrintHumanReadableCallstackToDebugger( msg->callstack );
	}
#endif
}


//--------------------------------------------------------------------------------------------------------------
bool Logger::IsFilteredOut( const char* tag )
{
	bool foundInFilters = m_activeFilters->Find( tag, nullptr );

	//Collapsing branching code into less straightforward predicates in case of speed-critical per-frame logging:
	return ( foundInFilters == ( m_currentFilterMode == FILTER_MODE_BLACKLIST ) );
	//i.e. if both values are the same, we DO filter out the logger request.
		//If we're blacklisting and found it in filters (both true), filter out.
		//If we're whitelisting and did not find it in the filters (both false), filter out.
}


//--------------------------------------------------------------------------------------------------------------
void Logger::ProcessRemainingMessages()
{
	Message* msg = nullptr;
	while ( m_messageQueue->Dequeue( &msg ) )
	{
		HandleMessage( msg ); //Again, depending on the callback here, may need critical section.
		msg->~Message();
		free( msg );
	}
}


//--------------------------------------------------------------------------------------------------------------
void Logger::LoggerThreadEntry( void* )
{
	bool openedFile = false;

	//Can assume any write to a bool is safe. This outer loop means "while someone hasn't told me to quit":
	while ( Logger::ShouldRun() )
	{
		Message* msg = nullptr;
		if ( m_logFile == nullptr )
			openedFile = TryCreateFile( &m_logFile, LOG_FILE_PATH, "ab" ); //Can't use Error library if this fails, since it depends on Logger it would yield an infinite loop.

		if ( openedFile ) 
		{
			while ( m_messageQueue->Dequeue( &msg ) )
			{
				HandleMessage( msg ); //Depending on the callback here, may need critical section.
					//e.g. a callback that writes to the developer console would need more care to not cause race conditions.
					//Can't write to the developer console because rendering's on the main thread. 
					//Adding to the map of stored lines would not be safe.
					//Creating new rendering resources/meshes also not thread safe.
					//Instead, the developer console will have to check to see if the logger has flagged anything for it to write.
				msg->~Message();
				free( msg );
			}
		}
		Thread::ThreadYield(); //Lets other threads run on the CPU instead of this one if we have no messages to process.
	}
	ProcessRemainingMessages(); //Re-runs the above loop one last time, in case we were told to stop while messages are still queued.
	fclose( m_logFile );

	delete m_messageQueue;
	m_messageQueue = nullptr;

	delete m_activeFilters;
	m_activeFilters = nullptr;
}


//--------------------------------------------------------------------------------------------------------------
void Logger::Startup()
{
	m_isRunning = true; 
	m_ioThread = new Thread( Logger::LoggerThreadEntry );
	m_messageQueue = new ThreadSafeQueue<Message*>();
	m_activeFilters = new ThreadSafeVector<const char*>();
}


//--------------------------------------------------------------------------------------------------------------
void Logger::Flush()
{
	ProcessRemainingMessages();
	fflush( m_logFile ); //Most file I/O operations like this are thread-safe (can confer MSDN page).
}


//--------------------------------------------------------------------------------------------------------------
void Logger::ToggleFilterMode()
{
	if ( m_currentFilterMode == FILTER_MODE_BLACKLIST )
	{
		m_currentFilterMode = FILTER_MODE_WHITELIST;
		g_theConsole->Printf( "Logger now interpreting filters as a whitelist." );
	}
	else
	{
		m_currentFilterMode = FILTER_MODE_BLACKLIST;
		g_theConsole->Printf( "Logger now interpreting filters as a blacklist." );
	}
}


//--------------------------------------------------------------------------------------------------------------
void Logger::AddFilter( const char* tag )
{
	if ( m_activeFilters->Find( tag, nullptr ) )
	{
		g_theConsole->Printf( "Filter already active." );
		return;
	}

	size_t length = strlen( tag );
	char* tagCopy = (char*)malloc( length+1 ); //+1 for the null after the strlen's amount.
	strcpy_s( tagCopy, length+1, tag );
	m_activeFilters->PushBack( tagCopy );
}

//--------------------------------------------------------------------------------------------------------------
void Logger::RemoveFilter( const char* tag )
{
	std::vector<const char*, UntrackedAllocator<const char*> >::iterator outPosition;
	bool found = m_activeFilters->Find( tag, &outPosition );
	if ( !found )
	{
		g_theConsole->Printf( "Filter not found in current %s.", ( m_currentFilterMode == FILTER_MODE_BLACKLIST ) ? "blacklist" : "whitelist" );
		return;
	}

	void* filterName = (void*)(*outPosition);
	free( filterName );
	m_activeFilters->Erase( outPosition, nullptr );
}


//--------------------------------------------------------------------------------------------------------------
void Logger::RemoveAllFilters()
{
	std::vector<const char*, UntrackedAllocator<const char*> >::iterator outPosition;
	size_t numFilters = m_activeFilters->Size();
	for ( unsigned int index = 0; index < numFilters; index++ )
	{
		const char* out;
		m_activeFilters->At( index, &out );
		void* filterName = (void*)out;
		free( filterName );
	}

	m_activeFilters->Clear();
}


//--------------------------------------------------------------------------------------------------------------
void Logger::Printf( const char* messageFormat, ... )
{
	va_list variableArgumentList;
	va_start( variableArgumentList, messageFormat );
	vaPrint( DEFAULT_FILTER, messageFormat, variableArgumentList );
	//va_end inside above function.
}


//--------------------------------------------------------------------------------------------------------------
void Logger::PrintfWithTag( const char* tag, const char* messageFormat, ... )
{
	va_list variableArgumentList;
	va_start( variableArgumentList, messageFormat );
	vaPrint( tag, messageFormat, variableArgumentList );
	//va_end inside above function.
}


//--------------------------------------------------------------------------------------------------------------
void Logger::PrintfWithCallstack( const char* messageFormat, ... )
{
	va_list variableArgumentList;
	va_start( variableArgumentList, messageFormat );
	vaPrint( DEFAULT_FILTER, messageFormat, variableArgumentList );
	//va_end inside above function.
}


//--------------------------------------------------------------------------------------------------------------
void Logger::PrintfWithTagAndCallstack( const char* tag, const char* messageFormat, ... )
{
	va_list variableArgumentList;
	va_start( variableArgumentList, messageFormat );
	vaPrint( tag, messageFormat, variableArgumentList, true );
	//va_end inside above function.
}


//--------------------------------------------------------------------------------------------------------------
void Logger::EnqueueMessage( const char* messageLiteral, int messageLength, bool includeCallstack /*= false*/ )
{
	//NOTE: because of the malloc + placement new combo, must now call dtor explicitly followed by free() in logger's async loop, not just delete.
	Message* msg = (Message*)malloc( sizeof( Message ) );
	new( msg ) Message( messageLiteral, messageLength ); //+1 because we added \n to the end.

	if ( includeCallstack )
		msg->callstack = Callstack::FetchAndAllocate( NUM_IGNORED_STACK_FRAMES );

	m_messageQueue->Enqueue( msg );
}


//--------------------------------------------------------------------------------------------------------------
void Logger::vaPrint( const char* tag, const char* messageFormat, va_list variableArgumentList, bool includeCallstack /*= false*/ )
{
	if ( IsFilteredOut( tag ) )
		return;

	const int MESSAGE_MAX_LENGTH = 2048;
	char taggedMessageFormat[ MESSAGE_MAX_LENGTH ] = "[";
	strcat_s( taggedMessageFormat, tag );
	strcat_s( taggedMessageFormat, "] " );
	strcat_s( taggedMessageFormat, messageFormat );

	char messageLiteral[ MESSAGE_MAX_LENGTH ];
	int charsWritten = vsnprintf_s( messageLiteral, MESSAGE_MAX_LENGTH, _TRUNCATE, taggedMessageFormat, variableArgumentList );
	va_end( variableArgumentList );
	messageLiteral[ charsWritten ] = '\n';
	messageLiteral[ charsWritten + 1 ] = '\0';
	messageLiteral[ MESSAGE_MAX_LENGTH - 1 ] = '\0'; // In case vsnprintf overran, since then it doesn't guarantee an ending null-char.

	EnqueueMessage( messageLiteral, charsWritten + 1, includeCallstack );
}
