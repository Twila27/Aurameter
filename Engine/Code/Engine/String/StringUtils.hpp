//Stringf By Squirrel Eiserloh

#pragma once
//-----------------------------------------------------------------------------------------------
#include <string>
#include <vector>

//-----------------------------------------------------------------------------------------------
//Don't forget to use std::to_string(val) for primitives!
const std::string Stringf( const char* format, ... ); //Use this most often.
const std::string Stringf( const int maxLength, const char* format, ... ); //For 1000s+ chars.
const std::string GetAsLowercase( const std::string& mixedCaseString );
const std::string GetAsUppercase( const std::string& mixedCaseString );
const std::string ReplaceAllOccurrencesInStringOfSubstr( const std::string& stringToEdit, const std::string& substrToReplace, const std::string& substrToReplaceby );
std::vector< std::string > SplitString( const char* fullString, char delimiter = '\n', bool ignoreTabs = true, bool includeNullChar = true );


//-----------------------------------------------------------------------------
//Base version.
//
template< typename WrappableType >
inline std::string GetTypedObjectAsString( WrappableType value )
{
	return value.ToString(); //For non-primitives, they have to have a ToString() on them.
}


//-----------------------------------------------------------------------------
//Primitive specializations. MUST BE INLINED, or it throws LNK4006.
//
template<>
inline std::string GetTypedObjectAsString<std::string>( std::string value )
{
	return value; //Follow for primitives.
}

template<>
inline std::string GetTypedObjectAsString<float>( float value )
{
	return std::to_string( value ); //Follow for primitives.
}

template<>
inline std::string GetTypedObjectAsString<int>( int value )
{
	return std::to_string( value ); //Follow for primitives.
}

template<>
inline std::string GetTypedObjectAsString<char>( char value )
{
	std::string out;
	out += value;
	return out;
}


//-----------------------------------------------------------------------------
//Base version.
//
template< typename UnwrappedType >
inline void SetTypeFromUnwrappedString( UnwrappedType& destination_out, const std::string& asString )
{
	const UnwrappedType constructedFromString( asString ); //MUST HAVE CTOR(const std::string&)!
	destination_out = constructedFromString;
}


//-----------------------------------------------------------------------------
//Primitive specializations. MUST BE INLINED, or it throws LNK4006.
//
template<>
inline void SetTypeFromUnwrappedString<float>( float& destination_out, const std::string& asString )
{
	destination_out = static_cast<float>( atof( asString.c_str() ) );
}

template<>
inline void SetTypeFromUnwrappedString<int>( int& destination_out, const std::string& asString )
{
	destination_out = static_cast<int>( atoi( asString.c_str() ) );
}

template<>
inline void SetTypeFromUnwrappedString<char>( char& destination_out, const std::string& asString )
{
	destination_out = ( asString == "" ) ? NULL : destination_out = static_cast<char>( asString.at( 0 ) );
}


//-----------------------------------------------------------------------------
//Pointer version.
//
template< typename UnwrappedType >
void SetTypeFromUnwrappedString( UnwrappedType* destination_out, const std::string& asString )
{
	(void)( asString ); //Unreferenced parameter.
		
	destination_out = NULL;
}