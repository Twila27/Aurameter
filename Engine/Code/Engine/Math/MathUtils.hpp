//Credit to Squirrel Eiserloh whose code helped me overhaul and add to much of the below.
#pragma once
#include <math.h>
#include <stdlib.h>
#include "Engine/EngineCommon.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"

//-----------------------------------------------------------------------------------------------
// Constants
//
const double TWO_PI			= 6.283185307179586476925286766559;
const double PI				= 3.1415926535897932384626433832795;
const double HALF_PI		= 1.5707963267948966192313216916398;
const double QUARTER_PI		= 0.78539816339744830961566084581988;
const double SQRT_2			= 1.4142135623730950488016887242097;
const double SQRT_3			= 1.7320508075688772935274463415059;
const double SQRT_2_OVER_2	= 0.70710678118654752440084436210485;
const double SQRT_3_OVER_2	= 0.86602540378443864676372317075294;
const double SQRT_3_OVER_3	= 0.57735026918962576450914878050196;
const double SQRT_3_OVER_6	= 0.28867513459481288225457439025098;

const float fTWO_PI			= 6.283185307179586476925286766559f;
const float fPI				= 3.1415926535897932384626433832795f;
const float fHALF_PI		= 1.5707963267948966192313216916398f;
const float fQUARTER_PI		= 0.78539816339744830961566084581988f;
const float fSQRT_2			= 1.4142135623730950488016887242097f;
const float fSQRT_3			= 1.7320508075688772935274463415059f;
const float fSQRT_2_OVER_2	= 0.70710678118654752440084436210485f;
const float fSQRT_3_OVER_2	= 0.86602540378443864676372317075294f;
const float fSQRT_3_OVER_3	= 0.57735026918962576450914878050196f;
const float fSQRT_3_OVER_6	= 0.28867513459481288225457439025098f;


//-----------------------------------------------------------------------------------------------
// Lookup tables
extern const float g_sinValuesForByteAngles[ 256 ];


//-----------------------------------------------------------------------------------------------
// Basic math utilities
//
template< typename T > const T& GetMin( const T& a, const T& b );
template< typename T > const T& GetMin( const T& a, const T& b, const T& c );
template< typename T > const T& GetMin( const T& a, const T& b, const T& c, const T& d );
template< typename T > const T& GetMax( const T& a, const T& b );
template< typename T > const T& GetMax( const T& a, const T& b, const T& c );
template< typename T > const T& GetMax( const T& a, const T& b, const T& c, const T& d );
template< typename T > const T GetMax( const T* values, size_t count );
template< typename T > T Clamp( const T& originalValue, const T& minValue, const T& maxValue );
template< typename T > T ClampZeroToOne( T value );
template< typename T > T ClampNegOneToOne( T value );
const int CalcLogBase2( int x ); //needs int, uses bit-shift.
const bool IsPowerOfTwo( int x ); //needs int, uses bit-shift.
template< typename T > T FastFloor( T f );
template< typename T > int FastFloorToInt( T f );

	
//-----------------------------------------------------------------------------
//Utility Math
template <typename T> T Interpolate( T start, T end, T fractionTowardEnd );
template <typename T> T RangeMap( T inValue, T inStart, T inEnd, T outStart, T outEnd );
float CalcFractionInRange( float inValue, float rangeStart, float rangeEnd );
float ClampFloat( float inValue, float min, float max );
int ClampInt( int inValue, int min, int max );
float ClampFloatZeroToOne( float inValue );
float ClampFloatNegativeOneToOne( float inValue );
float WrapNumberWithinCircularRange( float inValue, float minValue, float maxValue );
int WrapNumberWithinCircularRange( int inValue, int minValue, int maxValue );


//-----------------------------------------------------------------------------
//Convenience Math
template <typename T> T Atan2Degrees( const Vector2<T>& direction );
template <typename T> T DotProduct( const Vector2<T>& lhs, const Vector2<T>& rhs );
template <typename T> T DotProduct( const Vector3<T>& lhs, const Vector3<T>& rhs );
template <typename T> T DotProduct( const Vector4<T>& lhs, const Vector4<T>& rhs );
template <typename T> Vector3<T> CrossProduct( const Vector3<T>& lhs, const Vector3<T>& rhs );

//-----------------------------------------------------------------------------------------------
// Angle and Trig utilities
// 
template <typename T> T ConvertRadiansToDegrees( T radians );
template <typename T> T ConvertDegreesToRadians( T degrees );
template <typename T> T CosDegrees(	T degrees );
template <typename T> T SinDegrees( T degrees );
template <typename T> T CalcShortestAngularDeltaDegrees( T startDegrees, T endDegrees );
template <typename T> const Vector3<T> SineSquared( const Vector2<T>& position, const void* );

//-----------------------------------------------------------------------------
//Collision Math
double CalcDistBetweenPoints( const Vector2<double>& pos1, const Vector2<double>& pos2 );
float CalcDistBetweenPoints( const Vector2f& pos1, const Vector2f& pos2 );
float CalcDistBetweenPoints( const Vector2i& pos1, const Vector2i& pos2 );
template <typename T> float CalcManhattanDistBetweenPoints( const Vector2<T>& pos1, const Vector2<T>& pos2 );
float CalcDistSquaredBetweenPoints( const Vector2f& pos1, const Vector2f& pos2 );
int CalcDistSquaredBetweenPoints( const Vector2i& pos1, const Vector2i& pos2 );
float CalcShortestAngularDisplacement( float fromDegrees, float toDegrees );
bool DoDiscsOverlap( const Vector2f& center1, float radius1, const Vector2f& center2, float radius2 );
template <typename T> bool DoAABBsOverlap( const AABB2<T>& box1, const AABB2<T>& box2 );
bool IsPointInDisc( const Vector2f& point, const Vector2f& discCenter, float discRadius );
template <typename T> bool IsPointInAABB( const Vector2<T>& point, const AABB2<T>& box );


//-----------------------------------------------------------------------------------------------
// Interpolation/easing/tweening utilities
//
float SmoothStep( float inputZeroToOne );
float SmoothStep5( float inputZeroToOne );
float SmoothStart( float inputZeroToOne );
float SmoothStop( float inputZeroToOne );


//-----------------------------------------------------------------------------------------------
// Random number utilities
//
float GetRandomFloatZeroTo( float maximum );
template <typename T> T GetRandomElementInRange( T minInclusive, T maxInclusive );
int GetRandomIntInRange( float minInclusive, float maxInclusive );
int GetRandomIntInRange( int minInclusive, int maxInclusive );
int GetRandomIntLessThan( int maxExclusive );
float GetRandomFloatInRange( float minimumInclusive, float maximumInclusive );
bool GetRandomChance( float chanceForSuccess );


/////////////////////////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------------------------
template< typename T >
const T& GetMin( const T& a, const T& b )
{
	return a < b ? a : b;
}


//-----------------------------------------------------------------------------------------------
template< typename T >
const T& GetMin( const T& a, const T& b, const T& c )
{
	if ( a < b )
	{
		return a < c ? a : c;
	}
	else
	{
		return b < c ? b : c;
	}
}


//-----------------------------------------------------------------------------------------------
template< typename T >
const T& GetMin( const T& a, const T& b, const T& c, const T& d )
{
	if ( a < b )
	{
		if ( a < c )
		{
			return a < d ? a : d;
		}
		else
		{
			return c < d ? c : d;
		}
	}
	else
	{
		if ( b < c )
		{
			return b < d ? b : d;
		}
		else
		{
			return c < d ? c : d;
		}
	}
}


//-----------------------------------------------------------------------------------------------
template< typename T >
const T& GetMax( const T& a, const T& b )
{
	return a > b ? a : b;
}


//-----------------------------------------------------------------------------------------------
template< typename T >
const T& GetMax( const T& a, const T& b, const T& c )
{
	if ( a > b )
	{
		return a > c ? a : c;
	}
	else
	{
		return b > c ? b : c;
	}
}


//-----------------------------------------------------------------------------------------------
template< typename T >
const T& GetMax( const T& a, const T& b, const T& c, const T& d )
{
	if ( a > b )
	{
		if ( a > c )
		{
			return a > d ? a : d;
		}
		else
		{
			return c > d ? c : d;
		}
	}
	else
	{
		if ( b > c )
		{
			return b > d ? b : d;
		}
		else
		{
			return c > d ? c : d;
		}
	}
}


//-----------------------------------------------------------------------------------------------
template< typename T > 
const T GetMax( const T* values, size_t count )
{
	T currentMax = values[ 0 ];

	for ( unsigned int i = 1; i < count; ++i )
	{
		if ( currentMax < values[ i ] )
			currentMax = values[ i ];
	}
	
	return currentMax;
}


//-----------------------------------------------------------------------------------------------
template <typename T> 
inline const Vector3<T> SineSquared( const Vector2<T>& position, const void* )
{ 
	return Vector3<T>( position.x, position.y, sin( position.x*position.y ) );
}


//-----------------------------------------------------------------------------------------------
template< typename T >
inline T Clamp( const T& originalValue, const T& minValue, const T& maxValue )
{
	T clampedValue = originalValue;
	if( clampedValue < minValue )
		clampedValue = minValue;
	else if( clampedValue > maxValue )
		clampedValue = maxValue;

	return clampedValue;
}


//-----------------------------------------------------------------------------------------------
template< typename T >
inline T ClampZeroToOne( T value )
{
	Clamp( value, static_cast<T>( 0.f ), static_cast<T>( 1.f ) );
}


//-----------------------------------------------------------------------------------------------
template< typename T >
inline float ClampNegOneToOne( T value )
{
	Clamp( value, static_cast<T>( -1.f ), static_cast<T>( 1.f ) );
}


//-----------------------------------------------------------------------------------------------
template< typename T >
inline T ConvertRadiansToDegrees( T radians )
{
	return radians * static_cast<T>(180.0 / PI);
}


//-----------------------------------------------------------------------------------------------
template< typename T >
inline T ConvertDegreesToRadians( T degrees )
{
	return degrees * static_cast<T>(PI / 180.0);
}


//-----------------------------------------------------------------------------------------------
template< typename T >
inline T CosDegrees( T degrees )
{
	return cosf( ConvertDegreesToRadians( degrees ) );
}


//-----------------------------------------------------------------------------------------------
template< typename T >
inline T SinDegrees( T degrees )
{
	return sinf( ConvertDegreesToRadians( degrees ) );
}


//-----------------------------------------------------------------------------------------------
template< typename T >
inline T CalcShortestAngularDeltaDegrees( T startDegrees, T endDegrees )
{
	T angularDisplacementDegrees = endDegrees - startDegrees;
	while( angularDisplacementDegrees > static_cast<T>(180.f ) )
		angularDisplacementDegrees -= static_cast<T>( 360.f );
	while( angularDisplacementDegrees < static_cast<T>( -180.f ) )
		angularDisplacementDegrees += static_cast<T>( 360.f );

	return angularDisplacementDegrees;
}


//-----------------------------------------------------------------------------------------------
// For a given positive power-of-two x, find L = CalcLogBase2( x ) such that 2^L = X.
//
// For example, CalcLogBase2( 32 ) = 5, since 2^5 = 32.
//
inline const int CalcLogBase2( int x )
{
	int numBitShifts = 0;
	while( x )
	{
		x >>= 1;
		++ numBitShifts;
	}

	return numBitShifts-1; // e.g. if we shifted 6 times right before zeroing out, we were 00100000 (32), which is 2^5 (or 1<<5).
}


//-----------------------------------------------------------------------------------------------
// Returns true if x is a positive power of two (e.g. 1, 2, 4, 8, 16, 32, 64, 128, 256, 512...)
//
inline const bool IsPowerOfTwo( int x )
{
	return x && !(x & (x - 1));
}


//-----------------------------------------------------------------------------------------------
inline float SmoothStep( float inputZeroToOne )
{
	return inputZeroToOne * inputZeroToOne * (3.f - (2.f * inputZeroToOne));
}	


//-----------------------------------------------------------------------------------------------
inline float SmoothStep5( float inputZeroToOne )
{
	const float& t = inputZeroToOne;
	return t * t * t * ( t * (t * 6.f - 15.f) + 10.f );
}	


//-----------------------------------------------------------------------------------------------
inline float SmoothStart( float inputZeroToOne )
{
	return (inputZeroToOne * inputZeroToOne);
}	


//-----------------------------------------------------------------------------------------------
inline float SmoothStop( float inputZeroToOne )
{
	float oneMinusInput = 1.f - inputZeroToOne;
	return 1.f - (oneMinusInput * oneMinusInput);
}	


//-----------------------------------------------------------------------------------------------
inline int GetRandomIntInRange( int minValueInclusive, int maxValueInclusive )
{
	TODO( "Use a faster, better random number generator." );
	TODO( "Use more bits for higher-range numbers." );
	return minValueInclusive + rand() % ( 1 + maxValueInclusive - minValueInclusive );
}


//-----------------------------------------------------------------------------------------------
inline int GetRandomIntLessThan( int maxValueNotInclusive )
{
	TODO( "Use a faster, better random number generator." );
	TODO( "Use more bits for higher-range numbers." );
	return rand() % maxValueNotInclusive;
}


//-----------------------------------------------------------------------------------------------
inline float GetRandomFloatZeroTo( float maximumInclusive )
{
	TODO( "Use a faster, better random number generator." );
	const float oneOverRandMax = 1.f / static_cast< float >( RAND_MAX / maximumInclusive );
	return static_cast< float >( rand() ) * oneOverRandMax;
}


//-----------------------------------------------------------------------------------------------
template <typename T> inline T GetRandomElementInRange( T minInclusive, T maxInclusive )
{
	TODO( "Use a faster, better random number generator." );
	const float randomZeroToOne = GetRandomFloatZeroTo( 1.f );
	return minInclusive + static_cast<T>( ( maxInclusive - minInclusive ) * randomZeroToOne );
}


//-----------------------------------------------------------------------------------------------
template <> inline Vector2f GetRandomElementInRange( Vector2f minInclusive, Vector2f maxInclusive )
{
	TODO( "Use a faster, better random number generator." );
	const float randomZeroToOneX = GetRandomFloatZeroTo( 1.f );
	const float randomZeroToOneY = GetRandomFloatZeroTo( 1.f );
	return minInclusive + Vector2f( ( maxInclusive.x - minInclusive.x ) * randomZeroToOneX, ( maxInclusive.y - minInclusive.y ) * randomZeroToOneY );
}


//-----------------------------------------------------------------------------------------------
inline float GetRandomFloatInRange( float minimumInclusive, float maximumInclusive )
{
	TODO( "Use a faster, better random number generator." );
	const float randomZeroToOne = GetRandomFloatZeroTo( 1.f );
	return minimumInclusive + ( randomZeroToOne * ( maximumInclusive - minimumInclusive ) );
}


//-----------------------------------------------------------------------------------------------
inline bool GetRandomChance( float probabilityOfReturningTrue )
{
	if( probabilityOfReturningTrue >= 1.f )
		return true;

	return GetRandomFloatZeroTo( 1.f ) < probabilityOfReturningTrue;
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
T Interpolate( T start, T end, T fractionTowardEnd )
{
	return ( ( end - start ) * fractionTowardEnd ) + start;
}


//--------------------------------------------------------------------------------------------------------------
//Map inValue, from the range inStart to inEnd, to the new range outStart to outEnd.
//
template <typename T>
inline T RangeMap( T inValue, T inStart, T inEnd, T outStart, T outEnd )
{
	if ( inStart == inEnd )
		ERROR_AND_DIE( "RangeMap Division by Zero: inStart == inEnd" );


	T inLength = inEnd - inStart;
	T outLength = outEnd - outStart;
	T inValueToStartLength = inValue - inStart;
	T inValueLengthAlongRange = inValueToStartLength / inLength;

	return ( inValueLengthAlongRange * outLength ) + outStart;
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
T DotProduct( const Vector3<T>& lhs, const Vector3<T>& rhs )
{
	return ( lhs.x * rhs.x ) + ( lhs.y * rhs.y ) + ( lhs.z * rhs.z );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
Vector3<T> CrossProduct( const Vector3<T>& lhs, const Vector3<T>& rhs )
{
	float x = ( lhs.y * rhs.z ) - ( lhs.z * rhs.y );
	float y = ( lhs.z * rhs.x ) - ( lhs.x * rhs.z );
	float z = ( lhs.x * rhs.y ) - ( lhs.y * rhs.x );


	return Vector3<T>( x, y, z );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T> float CalcManhattanDistBetweenPoints( const Vector2<T>& pos1, const Vector2<T>& pos2 )
{
	return static_cast<float>( abs( pos2.x - pos1.x ) + abs( pos2.y - pos1.y ) );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
inline T Lerp( const T& a, const T& b, float t )
{
	return ( a * ( 1.f - t ) ) + ( b * t );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
inline T Slerp( const T& start, const T& end, float t )
{
	float startLength = start.CalcFloatLength();
	float endLength = end.CalcFloatLength();

	float slerpLength = Lerp( startLength, endLength, t );
	Vector3f slerpUnit = SlerpUnit( start / startLength, end / endLength, t );
	return slerpUnit * slerpLength;
}


//--------------------------------------------------------------------------------------------------------------
//TODO( "Move into MathUtils, just keep Vector3<T>" );
template <typename T>
inline T SlerpUnit( const T& start, const T& end, float t )
{
	float cosAngle = ClampFloat( DotProduct( start, end ), -1.f, 1.f );
	float angle = acos( ConvertDegreesToRadians( cosAngle ) );

	if ( angle < std::numeric_limits<float>::epsilon() )
	{
		return Lerp( start, end, t );
	}
	else
	{
		float positive = SinDegrees( t * angle );
		float negative = SinDegrees( ( 1.f - t ) * angle );
		float denominator = SinDegrees( angle );

		return ( ( start * ( negative / denominator ) ) + ( end * ( positive / denominator ) ) );
	}
}