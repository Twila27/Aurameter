// Stringf By Squirrel Eiserloh

#include "Engine/EngineCommon.hpp"
#include "Engine/String/StringUtils.hpp"
#include <stdarg.h>


//-----------------------------------------------------------------------------------------------
const int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const int maxLength, const char* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}


//-----------------------------------------------------------------------------------------------
const std::string GetAsLowercase( const std::string& mixedCaseString )
{
	std::string copyToTurnLowercase = mixedCaseString;

	auto charIterEnd = copyToTurnLowercase.end();
	for ( auto charIter = copyToTurnLowercase.begin( ); charIter != charIterEnd; charIter++ )
	{
		int uncapitalizedCharAsInt = tolower( *charIter );
		*charIter = static_cast<char>( uncapitalizedCharAsInt );
	}

	return copyToTurnLowercase;
}


//-----------------------------------------------------------------------------------------------
const std::string GetAsUppercase( const std::string& mixedCaseString )
{
	std::string copyToTurnUppercase = mixedCaseString;

	auto charIterEnd = copyToTurnUppercase.end();
	for ( auto charIter = copyToTurnUppercase.begin(); charIter != charIterEnd; charIter++ )
	{
		int capitalizedCharAsInt = toupper( *charIter );
		*charIter = static_cast<char>( capitalizedCharAsInt );
	}

	return copyToTurnUppercase;
}


//-----------------------------------------------------------------------------------------------
const std::string ReplaceAllOccurrencesInStringOfSubstr( const std::string& stringToEdit, const std::string& substrToReplace, const std::string& substrToReplaceBy )
{
	std::string tmp = stringToEdit;
	unsigned int sizeOfReplacedTerm = substrToReplace.size();
	unsigned int sizeOfReplacement = substrToReplaceBy.size();
	unsigned int index = 0;

	while ( true )
	{
		index = tmp.find( substrToReplace, index );
		if ( index == std::string::npos )
			break;

		tmp.erase( index, sizeOfReplacedTerm );
		tmp.insert( index, substrToReplaceBy );
//		tmp.replace( index, sizeOfReplacement, substrToReplaceBy ); //This actually overwrites, not as desired.

		index += sizeOfReplacement;
	}

	return tmp;
}


//--------------------------------------------------------------------------------------------------------------
std::vector< std::string > SplitString( const char* fullString, char delimiter /*= '\n'*/, bool ignoreTabs /*= true*/, bool includeNullChar /*= true*/ )
{
	//Checks to ensure delimiters aren't ignored.
	ignoreTabs = ignoreTabs && ( delimiter != '\t' ); //Shuts back to false if delimiting on tabs, even if ignoreTabs is true.
	bool ignoreReturns = ( delimiter != '\r' );

	std::vector< std::string > splitStrings;
	splitStrings.push_back( "" );
	unsigned int charIndex = 0;
	char currentChar; //while clause's check requires outside declaration.

	do
	{
		currentChar = fullString[ charIndex++ ];

		//Any unimportant characters ignored here.
		TODO( "Add a param string of characters to ignore, check against != delimiter as above, then loop over that vs current char here to give utility free reign over ignoring." );
		if ( ignoreReturns && currentChar == '\r' )
			continue;
		if ( ignoreTabs && currentChar == '\t' )
			continue;

		if ( currentChar == delimiter )
		{
			splitStrings.push_back( "" );
			continue;
		}

		if ( ( currentChar != '\0' ) || includeNullChar )
			splitStrings.back() += currentChar;

	} while ( currentChar != '\0' );

	return splitStrings;
}