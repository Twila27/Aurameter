#pragma once
#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/String/StringUtils.hpp"
#include "Engine/EngineCommon.hpp"
#include <map>
#include <vector>


//--------------------------------------------------------------------------------------------------------------
struct Rgba;


//--------------------------------------------------------------------------------------------------------------
class Command
{

public:

	Command( const std::string& fullCommandString ); //See RunCommand().
	std::string GetCommandName() const { return m_commandName; }
	std::string GetArgsString() const { return m_argsString; }
	bool GetNextString( std::string* out, const std::string* defaultValue = nullptr );
	bool GetNextColor( Rgba* out, Rgba defaultValue );
	bool GetNextFloat( float* out, float defaultValue );
	bool GetNextInt( int* out, int defaultValue );
	bool GetNextChar( char* out, char defaultValue );
	//etc... every GetNext follows the skeleton in GetNextColor.
	TODO( "bool GetNextInt( int* out, const char* defaultValue ) const;" );
	bool ParseColor( Rgba* out, const char* arg ) const;
	bool ParseFloat( float* out, const char* arg ) const;
	bool ParseInt( int* out, const char* arg ) const;
	bool ParseChar( char* out, const char* arg ) const;


private:

	std::string m_commandName;
	std::string m_argsString;
	std::vector< std::string > m_argsList;
	unsigned int m_currentArgsListPos;
};
