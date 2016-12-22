#include "Engine/Core/Command.hpp"
#include "Engine/Renderer/Rgba.hpp"
#include "Engine/String/StringUtils.hpp"


//--------------------------------------------------------------------------------------------------------------
Command::Command( const std::string& fullCommandString )
	: m_currentArgsListPos( 0 )
{
	//Split fullCommandString, e.g. "SetColor .5 .5 .3 .2" into name "SetColor" and argsString/argsList.
	m_commandName = "";
	m_argsString = "";
	char commandName[ 80 ];
	char argsString[ 400 ];

	int scanForNameResult = sscanf_s( fullCommandString.c_str(), "%s", commandName, _countof( commandName ) );
	if ( scanForNameResult < 1 ) //Didn't match all args or returned -1 for EOF.
	{
		DebuggerPrintf( "Sscanf_s Failure for Name in Command()\n" );
	}
	else
	{
		m_commandName = commandName;
	}
	
	//WARNING: if the args contain chars not in this regex, will cut off!!! The karat means up to and excluding what follows it.
	int scanForArgsResult = sscanf_s( fullCommandString.c_str(), "%*s %[^\n]", argsString, _countof(argsString) );
	if ( scanForArgsResult < 1 ) //Didn't match all args or returned -1 for EOF.
	{
		DebuggerPrintf( "Sscanf_s Found No Args in Command()\n" );
	}
	else
	{
		m_argsString = argsString;

		std::string currentFormat = "%s";
		char nextArg[ 80 ];
		scanForArgsResult = sscanf_s( m_argsString.c_str(), currentFormat.c_str(), nextArg, _countof(nextArg) );
		while ( scanForArgsResult > 0 ) //Until we no longer match.
		{
			m_argsList.push_back( nextArg );

			currentFormat.insert( 0, "%*s " );

			scanForArgsResult = sscanf_s( m_argsString.c_str(), currentFormat.c_str(), nextArg, _countof(nextArg) );
		}
	}
}


//  Example use of args:
// 	int arg0;
// 	const char* arg1;

// 	if ( args.GetNextInt( &arg0, 0 ) && args.GetNextString( &arg1, nullptr ) )
// 	{
// 		//Success! Do something with these two args.
// 	}
// 	else
// 	{
// 		DebuggerPrintf( "help follows format: help <int> <string>\n" );
// 	}
// 
// 	Rgba color;
// 	args.GetNextColor( &color, Rgba::WHITE ); //Second arg for default if &color returns with zilch.


//--------------------------------------------------------------------------------------------------------------
bool Command::GetNextString( std::string* out, const std::string* defaultValue /*= nullptr*/ )
{
	std::string arg;

	if ( m_currentArgsListPos < m_argsList.size() )
	{
		*out = m_argsList[ m_currentArgsListPos++ ];
		return true;
	}

	//If we hit here, failed.
	if ( defaultValue == nullptr )
		*out = "";
	else
		*out = *defaultValue;

	return false;
}


//--------------------------------------------------------------------------------------------------------------
bool Command::GetNextColor( Rgba* out, Rgba defaultValue )
{
	std::string arg;
	bool doesArgExist = ( m_argsString != "" ); //GetNextString( &arg, nullptr ); //Check m_argsList for another.
	if ( doesArgExist )
	{
		bool successfullyParsedAsColor = ParseColor( out, m_argsString.c_str() /*arg.c_str()*/ );
		if ( successfullyParsedAsColor )
			return true;
	}

	//If we hit here, failed, print usage.
	*out = defaultValue;
	return false;
}


//--------------------------------------------------------------------------------------------------------------
bool Command::GetNextFloat( float* out, float defaultValue )
{
	std::string arg;
	bool doesArgExist = GetNextString( &arg, nullptr ); //Check m_argsList for another.
	if ( doesArgExist )
	{
		bool successfullyParsedAsFloat = ParseFloat( out, arg.c_str() );
		if ( successfullyParsedAsFloat )
			return true;
	}

	//If we hit here, failed, print usage.
	*out = defaultValue;
	return false;
}


//--------------------------------------------------------------------------------------------------------------
bool Command::GetNextInt( int* out, int defaultValue )
{
	std::string arg;
	bool doesArgExist = GetNextString( &arg, nullptr ); //Check m_argsList for another.
	if ( doesArgExist )
	{
		bool successfullyParsedAsInt = ParseInt( out, arg.c_str() );
		if ( successfullyParsedAsInt )
			return true;
	}

	//If we hit here, failed, print usage.
	*out = defaultValue;
	return false;
}


//--------------------------------------------------------------------------------------------------------------
bool Command::GetNextChar( char* out, char defaultValue )
{
	std::string arg;
	bool doesArgExist = GetNextString( &arg, nullptr ); //Check m_argsList for another.
	if ( doesArgExist )
	{
		bool successfullyParsedAsInt = ParseChar( out, arg.c_str() );
		if ( successfullyParsedAsInt )
			return true;
	}

	//If we hit here, failed, print usage.
	*out = defaultValue;
	return false;
}


//--------------------------------------------------------------------------------------------------------------
bool Command::ParseColor( Rgba* out, const char* arg ) const
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
	int scanResult = sscanf_s( arg, " %hhu %hhu %hhu %hhu ", &r, &g, &b, &a ); //hhu means uint.
	if ( scanResult == 4 ) //4 matches.
	{
		*out = Rgba( r, g, b, a );
		return true;
	}

	return false;
}


//--------------------------------------------------------------------------------------------------------------
bool Command::ParseFloat( float* out, const char* arg ) const
{
	float f;
	int scanResult = sscanf_s( arg, " %f ", &f );
	if ( scanResult == 1 ) //4 matches.
	{
		*out = f;
		return true;
	}

	return false;
}


//--------------------------------------------------------------------------------------------------------------
bool Command::ParseInt( int* out, const char* arg ) const
{
	int i;
	int scanResult = sscanf_s( arg, " %d ", &i );
	if ( scanResult == 1 ) //4 matches.
	{
		*out = i;
		return true;
	}

	return false;
}


//--------------------------------------------------------------------------------------------------------------
bool Command::ParseChar( char* out, const char* arg ) const
{
	char c;
	int scanResult = sscanf_s( arg, " %c ", &c, 1 );
	if ( scanResult == 1 ) //4 matches.
	{
		*out = c;
		return true;
	}

	return false;
}