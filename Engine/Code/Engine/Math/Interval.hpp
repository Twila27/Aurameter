#pragma once
#include "Engine/Math/MathUtils.hpp"
#include "Engine/String/StringUtils.hpp"
#include <string>
#include <vector>


template < typename IntervalElement >
struct Interval
{
	Interval()
		: minInclusive()
		, maxInclusive()
	{
	}
	Interval( IntervalElement val )
		: minInclusive( val )
		, maxInclusive( val )
	{
	}
	Interval( IntervalElement minInclusive, IntervalElement maxInclusive )
		: minInclusive( minInclusive )
		, maxInclusive( maxInclusive )
	{
	}
	inline Interval( const std::string& stringToParse );
	inline bool operator==( const Interval<IntervalElement>& compareTo ) const { return ( minInclusive == compareTo.minInclusive ) && ( maxInclusive == compareTo.maxInclusive ); }
	inline std::string ToString() const;

	IntervalElement minInclusive;
	IntervalElement maxInclusive;

	inline IntervalElement GetRandomElement() const;
	inline IntervalElement Get( float t ) { return Lerp( minInclusive, maxInclusive, ClampFloatZeroToOne( t ) ); }
};


//--------------------------------------------------------------------------------------------------------------
template < typename IntervalElement > std::string Interval<IntervalElement>::ToString() const
{
	std::logic_error( "Unimplemented" );
}


//--------------------------------------------------------------------------------------------------------------
template <> std::string Interval<float>::ToString() const
{
	return Stringf( "%f~%f", minInclusive, maxInclusive );
}


//--------------------------------------------------------------------------------------------------------------
template <> std::string Interval<int>::ToString() const
{
	return Stringf( "%d~%d", minInclusive, maxInclusive );
}


//--------------------------------------------------------------------------------------------------------------
template < typename IntervalElement > inline Interval<IntervalElement>::Interval( const std::string& stringToParse )
{
	throw new std::logic_error( "Unimplemented string-unwrapping ctor template specialization in Interval class!" );
}


//--------------------------------------------------------------------------------------------------------------
template <> inline Interval<int>::Interval( const std::string& stringToParse )
{
	const char* cStringToParse = stringToParse.c_str();
	std::vector< std::string > stringSplitOnTilde = SplitString( cStringToParse, '~' );
	if ( stringSplitOnTilde.size() == 1 )
	{
		maxInclusive = minInclusive = atoi( cStringToParse );
	}
	else sscanf_s( cStringToParse, "%d~%d", &minInclusive, &maxInclusive );
}


//--------------------------------------------------------------------------------------------------------------
template <> inline Interval<float>::Interval( const std::string& stringToParse )
{
	const char* cStringToParse = stringToParse.c_str();
	std::vector< std::string > stringSplitOnTilde = SplitString( cStringToParse, '~' );
	if ( stringSplitOnTilde.size() == 1 )
	{
		maxInclusive = minInclusive = strtof( cStringToParse, NULL );
	}
	else sscanf_s( cStringToParse, "%f~%f", &minInclusive, &maxInclusive );
}


//--------------------------------------------------------------------------------------------------------------
template < typename IntervalElement > inline IntervalElement Interval<IntervalElement>::GetRandomElement() const
{
	return GetRandomElementInRange( minInclusive, maxInclusive );
}