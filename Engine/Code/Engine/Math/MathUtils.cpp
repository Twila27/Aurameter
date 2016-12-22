#include "Engine/Math/MathUtils.hpp"

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"

//-----------------------------------------------------------------------------------------------
// Lookup table for sin() values for 256 evenly distributed angles around the unit circle,
//	where Byte 0 = 0 degrees, Byte 64 = 90 degrees, Byte 128 = 180 degrees, etc.
//
const float g_sinValuesForByteAngles[ 256 ] = {
	0.000000000f,	0.024541229f,	0.049067674f,	0.073564564f,	0.098017140f,	0.122410675f,	0.146730474f,	0.170961889f,	
	0.195090322f,	0.219101240f,	0.242980180f,	0.266712757f,	0.290284677f,	0.313681740f,	0.336889853f,	0.359895037f,	
	0.382683432f,	0.405241314f,	0.427555093f,	0.449611330f,	0.471396737f,	0.492898192f,	0.514102744f,	0.534997620f,	
	0.555570233f,	0.575808191f,	0.595699304f,	0.615231591f,	0.634393284f,	0.653172843f,	0.671558955f,	0.689540545f,	
	0.707106781f,	0.724247083f,	0.740951125f,	0.757208847f,	0.773010453f,	0.788346428f,	0.803207531f,	0.817584813f,	
	0.831469612f,	0.844853565f,	0.857728610f,	0.870086991f,	0.881921264f,	0.893224301f,	0.903989293f,	0.914209756f,	
	0.923879533f,	0.932992799f,	0.941544065f,	0.949528181f,	0.956940336f,	0.963776066f,	0.970031253f,	0.975702130f,	
	0.980785280f,	0.985277642f,	0.989176510f,	0.992479535f,	0.995184727f,	0.997290457f,	0.998795456f,	0.999698819f,	
	1.000000000f,	0.999698819f,	0.998795456f,	0.997290457f,	0.995184727f,	0.992479535f,	0.989176510f,	0.985277642f,	
	0.980785280f,	0.975702130f,	0.970031253f,	0.963776066f,	0.956940336f,	0.949528181f,	0.941544065f,	0.932992799f,	
	0.923879533f,	0.914209756f,	0.903989293f,	0.893224301f,	0.881921264f,	0.870086991f,	0.857728610f,	0.844853565f,	
	0.831469612f,	0.817584813f,	0.803207531f,	0.788346428f,	0.773010453f,	0.757208847f,	0.740951125f,	0.724247083f,	
	0.707106781f,	0.689540545f,	0.671558955f,	0.653172843f,	0.634393284f,	0.615231591f,	0.595699304f,	0.575808191f,	
	0.555570233f,	0.534997620f,	0.514102744f,	0.492898192f,	0.471396737f,	0.449611330f,	0.427555093f,	0.405241314f,	
	0.382683432f,	0.359895037f,	0.336889853f,	0.313681740f,	0.290284677f,	0.266712757f,	0.242980180f,	0.219101240f,	
	0.195090322f,	0.170961889f,	0.146730474f,	0.122410675f,	0.098017140f,	0.073564564f,	0.049067674f,	0.024541229f,	
	0.000000000f,	-0.024541229f,	-0.049067674f,	-0.073564564f,	-0.098017140f,	-0.122410675f,	-0.146730474f,	-0.170961889f,	
	-0.195090322f,	-0.219101240f,	-0.242980180f,	-0.266712757f,	-0.290284677f,	-0.313681740f,	-0.336889853f,	-0.359895037f,	
	-0.382683432f,	-0.405241314f,	-0.427555093f,	-0.449611330f,	-0.471396737f,	-0.492898192f,	-0.514102744f,	-0.534997620f,	
	-0.555570233f,	-0.575808191f,	-0.595699304f,	-0.615231591f,	-0.634393284f,	-0.653172843f,	-0.671558955f,	-0.689540545f,	
	-0.707106781f,	-0.724247083f,	-0.740951125f,	-0.757208847f,	-0.773010453f,	-0.788346428f,	-0.803207531f,	-0.817584813f,	
	-0.831469612f,	-0.844853565f,	-0.857728610f,	-0.870086991f,	-0.881921264f,	-0.893224301f,	-0.903989293f,	-0.914209756f,	
	-0.923879533f,	-0.932992799f,	-0.941544065f,	-0.949528181f,	-0.956940336f,	-0.963776066f,	-0.970031253f,	-0.975702130f,	
	-0.980785280f,	-0.985277642f,	-0.989176510f,	-0.992479535f,	-0.995184727f,	-0.997290457f,	-0.998795456f,	-0.999698819f,	
	-1.000000000f,	-0.999698819f,	-0.998795456f,	-0.997290457f,	-0.995184727f,	-0.992479535f,	-0.989176510f,	-0.985277642f,	
	-0.980785280f,	-0.975702130f,	-0.970031253f,	-0.963776066f,	-0.956940336f,	-0.949528181f,	-0.941544065f,	-0.932992799f,	
	-0.923879533f,	-0.914209756f,	-0.903989293f,	-0.893224301f,	-0.881921264f,	-0.870086991f,	-0.857728610f,	-0.844853565f,	
	-0.831469612f,	-0.817584813f,	-0.803207531f,	-0.788346428f,	-0.773010453f,	-0.757208847f,	-0.740951125f,	-0.724247083f,	
	-0.707106781f,	-0.689540545f,	-0.671558955f,	-0.653172843f,	-0.634393284f,	-0.615231591f,	-0.595699304f,	-0.575808191f,	
	-0.555570233f,	-0.534997620f,	-0.514102744f,	-0.492898192f,	-0.471396737f,	-0.449611330f,	-0.427555093f,	-0.405241314f,	
	-0.382683432f,	-0.359895037f,	-0.336889853f,	-0.313681740f,	-0.290284677f,	-0.266712757f,	-0.242980180f,	-0.219101240f,	
	-0.195090322f,	-0.170961889f,	-0.146730474f,	-0.122410675f,	-0.098017140f,	-0.073564564f,	-0.049067674f,	-0.024541229f
};


//-----------------------------------------------------------------------------------------------
// A replacement for floor(), about 3x faster on my machine.
// Reliable within [-2 billion, +2 billion] or so.  I think.
//
template <typename T>
T FastFloor( T f )
{
	if( f >= 0.f )
		return (float) (int) f;
	else
	{
		float f2 = (float) (int) f;
		if( f == f2 )
			return f2;
		else
			return f2 - 1.f;
	}
}


//-----------------------------------------------------------------------------------------------
// A replacement for (int) floor(), about 3x faster on my machine
// Reliable within the range of int.  I think.
//
template <typename T>
int FastFloorToInt( T f )
{
	if( f >= 0.f )
		return (int) f;
	else
	{
		int i = (int) f;
		T f2 = (T) i;
		if( f == f2 )
			return i;
		else
			return i-1;
	}
}


//--------------------------------------------------------------------------------------------------------------
float CalcFractionInRange( float inValue, float rangeStart, float rangeEnd )
{
	if ( rangeEnd - rangeStart == 0 ) 
		ERROR_AND_DIE( "CalcFractionInRange Division by Zero: rangeStart == rangeEnd" );


	return (inValue - rangeStart) / (rangeEnd - rangeStart);
}


//--------------------------------------------------------------------------------------------------------------
float ClampFloat( float inValue, float min, float max )
{
	//In case the values came in backwards.
	float newMin = ( min < max ) ? min : max;
	float newMax = ( max > min ) ? max : min;

	if ( inValue < newMin ) return newMin;
	if ( inValue > newMax ) return newMax;
	return inValue;
}


//--------------------------------------------------------------------------------------------------------------
int ClampInt( int inValue, int min, int max )
{
	//In case the values came in backwards.
	int newMin = ( min < max ) ? min : max;
	int newMax = ( max > min ) ? max : min;

	if ( inValue < newMin ) return newMin;
	if ( inValue > newMax ) return newMax;
	return inValue;
}


//--------------------------------------------------------------------------------------------------------------
float ClampFloatZeroToOne( float inValue )
{
	return ClampFloat( inValue, 0.f, 1.f );
}


//--------------------------------------------------------------------------------------------------------------
float ClampFloatNegativeOneToOne( float inValue )
{
	return ClampFloat( inValue, -1.f, 1.f );
}


//--------------------------------------------------------------------------------------------------------------
float WrapNumberWithinCircularRange( float inValue, float minValue, float maxValue )
{
	float wrappedValue = inValue;
	float rangeSize = maxValue - minValue;
	
	while ( wrappedValue < minValue ) wrappedValue += rangeSize;
	while ( wrappedValue >= maxValue ) wrappedValue -= rangeSize;

	return wrappedValue;
}


//--------------------------------------------------------------------------------------------------------------
int WrapNumberWithinCircularRange( int inValue, int minValue, int maxValue )
{
	int wrappedValue = inValue;
	int rangeSize = maxValue - minValue;

	while ( wrappedValue < minValue ) wrappedValue += rangeSize;
	while ( wrappedValue >= maxValue ) wrappedValue -= rangeSize;

	return wrappedValue;
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
T Atan2Degrees( const Vector2<T>& direction )
{
	return ConvertRadiansToDegrees( atan2( direction.y, direction.x ) );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
T DotProduct( const Vector2<T>& lhs, const Vector2<T>& rhs )
{
	return ( lhs.x * rhs.x ) + ( lhs.y * rhs.y );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
T DotProduct( const Vector4<T>& lhs, const Vector4<T>& rhs )
{
	return ( lhs.x * rhs.x ) + ( lhs.y * rhs.y ) + ( lhs.z * rhs.z ) + ( lhs.w * rhs.w );
}


//--------------------------------------------------------------------------------------------------------------
double CalcDistBetweenPoints( const Vector2<double>& pos1, const Vector2<double>& pos2 )
{
	double xDistance = pos2.x - pos1.x;
	double yDistance = pos2.y - pos1.y;

	return sqrt( ( xDistance * xDistance ) + ( yDistance * yDistance ) );
}


//--------------------------------------------------------------------------------------------------------------
float CalcDistBetweenPoints( const Vector2f& pos1, const Vector2f& pos2 )
{
	float yDistance = pos2.y - pos1.y;
	float xDistance = pos2.x - pos1.x;

	return sqrt( ( xDistance * xDistance ) + ( yDistance * yDistance ) );
}


//--------------------------------------------------------------------------------------------------------------
float CalcDistBetweenPoints( const Vector2i& pos1, const Vector2i& pos2 )
{
	int xDistance = pos2.x - pos1.x;
	int yDistance = pos2.y - pos1.y;

	return sqrt( static_cast<float>( ( xDistance * xDistance ) + ( yDistance * yDistance ) ) );
}


//--------------------------------------------------------------------------------------------------------------
float CalcDistSquaredBetweenPoints( const Vector2f& pos1, const Vector2f& pos2 )
{
	float xDistance = pos2.x - pos1.x;
	float yDistance = pos2.y - pos1.y;

	return ( xDistance * xDistance ) + ( yDistance * yDistance );
}


//--------------------------------------------------------------------------------------------------------------
int CalcDistSquaredBetweenPoints( const Vector2i& pos1, const Vector2i& pos2 )
{
	int xDistance = pos2.x - pos1.x;
	int yDistance = pos2.y - pos1.y;

	return ( xDistance * xDistance ) + ( yDistance * yDistance );
}


//--------------------------------------------------------------------------------------------------------------
float CalcShortestAngularDisplacement( float fromDegrees, float toDegrees )
{
	float angularDisplacement = toDegrees - fromDegrees;

	while ( angularDisplacement > 180.f ) angularDisplacement -= 360.f;
	while ( angularDisplacement < -180.f ) angularDisplacement += 360.f;

	return angularDisplacement;
}


//--------------------------------------------------------------------------------------------------------------
bool DoDiscsOverlap( const Vector2f& center1, float radius1, const Vector2f& center2, float radius2 )
{
	float distanceSquared = CalcDistSquaredBetweenPoints( center1, center2 );
	float radiiSum = radius1 + radius2;

	return distanceSquared < ( radiiSum * radiiSum );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
bool IsPointInAABB( const Vector2<T>& point, const AABB2<T>& box )
{
	return (
		point.x >= box.mins.x &&
		point.y >= box.mins.y &&
		point.x <= box.maxs.x &&
		point.y <= box.maxs.y );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
bool DoAABBsOverlap( const AABB2<T>& box1, const AABB2<T>& box2 )
{
	T box1ActualMinX = GetMin( box1.mins.x, box1.maxs.x );
	T box2ActualMinX = GetMin( box2.mins.x, box2.maxs.x );
	T box1ActualMaxX = GetMax( box1.mins.x, box1.maxs.x );
	T box2ActualMaxX = GetMax( box2.mins.x, box2.maxs.x );

	T box1ActualMinY = GetMin( box1.mins.y, box1.maxs.y );
	T box2ActualMinY = GetMin( box2.mins.y, box2.maxs.y );
	T box1ActualMaxY = GetMax( box1.mins.y, box1.maxs.y );
	T box2ActualMaxY = GetMax( box2.mins.y, box2.maxs.y );

	if ( box1ActualMaxX < box2ActualMinX )
		return false; //box1 is left of box2.
	if ( box1ActualMinX > box2ActualMaxX )
		return false; //box1 is right of box2.
	if ( box1ActualMaxY < box2ActualMinY )
		return false; //box1 is below box2.
	if ( box1ActualMinY > box2ActualMaxY )
		return false; //box1 is above box2.
	return true;
}
template bool DoAABBsOverlap<int>( const AABB2<int>& box1, const AABB2<int>& box2 );
template bool DoAABBsOverlap<float>( const AABB2<float>& box1, const AABB2<float>& box2 );


//--------------------------------------------------------------------------------------------------------------
bool IsPointInDisc( const Vector2f& point, const Vector2f& discCenter, float discRadius )
{
	if ( ( point - discCenter ).CalcFloatLength() < discRadius ) return true;
	else return false;
}


//--------------------------------------------------------------------------------------------------------------
int GetRandomIntInRange( float minInclusive, float maxInclusive )
{
	return (int) floor( GetRandomFloatInRange( minInclusive, maxInclusive ) );
}

