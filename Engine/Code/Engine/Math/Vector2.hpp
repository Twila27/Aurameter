#pragma once


#include <string>
#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/String/StringUtils.hpp"


//-----------------------------------------------------------------------------
template < typename T >
class Vector2
{
public:

	inline Vector2();
	inline Vector2( T initialXY );
	inline Vector2( T initialX, T initialY );
	inline Vector2( const Vector2<T>& vectorToCopy );
	inline Vector2( const std::string& stringToParse );

	inline void GetXY( T& out_x, T& out_y ) const;
	inline void SetXY( T newX, T newY );

	inline float CalcFloatLength() const;
	inline float CalcFloatLengthSquared() const;
	inline double CalcDoubleLength() const;
	inline double CalcDoubleLengthSquared() const;
	inline void Normalize();

	inline const Vector2<T> operator+( const Vector2<T>& vectorToAdd ) const;
	inline const Vector2<T> operator-( const Vector2<T>& vectorToAdd ) const;
	inline const Vector2<T> operator-() const;
	inline const Vector2<T> operator*( const Vector2<T>& vectorToMultiplyComponentwise ) const;
	inline const Vector2<T> operator*( T scalarToScaleBy ) const;
	inline const Vector2<T> operator/( T scalarToScaleBy ) const;
	inline void operator+=( const Vector2<T>& vectorToAdd );
	inline void operator-=( const Vector2<T>& vectorToSubtract );
	inline void operator*=( const T uniformScale );
	inline void operator=( const Vector2<T>& copyFrom );
	inline bool operator<( const Vector2<T>& compareTo ) const;
	inline bool operator>( const Vector2<T>& compareTo ) const;
	inline bool operator==( const Vector2<T>& compareTo ) const;
	inline bool operator!=( const Vector2<T>& compareTo ) const;
	inline std::string ToString() const;

public: //Because Vector2 is virtually a primitive.

	static const Vector2<T> ZERO;
	static const Vector2<T> ONE;

	static const Vector2<T> UNIT_X;
	static const Vector2<T> UNIT_Y;

	T x;
	T y;
};
typedef Vector2<float> Vector2f;
typedef Vector2<int> Vector2i;


//--------------------------------------------------------------------------------------------------------------
// Do-nothing default ctor: because it saves time to leave trash values rather than allocate and initialize.
template < typename T >
inline Vector2<T>::Vector2()
	: x()
	, y()
{
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline Vector2<T>::Vector2( T initialX, T initialY )
	: x( initialX )
	, y( initialY )
{
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline Vector2<T>::Vector2( T initialXY )
	: x( initialXY )
	, y( initialXY )
{
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline Vector2<T>::Vector2( const Vector2<T>& vectorToCopy )
	: x( vectorToCopy.x )
	, y( vectorToCopy.y )
{
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline Vector2<T>::Vector2( const std::string& stringToParse )
{
	throw new std::logic_error( "Unimplemented constructor template instance in Vector2::stringToParse." );
}


//--------------------------------------------------------------------------------------------------------------
template <> inline Vector2<int>::Vector2( const std::string& stringToParse )
{
	sscanf_s( stringToParse.c_str(), "%d,%d", &x, &y );
}


//--------------------------------------------------------------------------------------------------------------
template <> inline Vector2<float>::Vector2( const std::string& stringToParse )
{
	sscanf_s( stringToParse.c_str(), "%f,%f", &x, &y );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline std::string Vector2<T>::ToString() const
{
	throw new std::logic_error( "Unimplemented constructor template instance in Vector2::stringToParse." );
}


//--------------------------------------------------------------------------------------------------------------
template <> inline std::string Vector2<int>::ToString() const
{
	return Stringf( "%d,%d", x, y );
}


//--------------------------------------------------------------------------------------------------------------
template <> inline  std::string Vector2<float>::ToString() const
{
	return Stringf( "%f,%f", x, y );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline void Vector2<T>::GetXY( T& out_x, T& out_y ) const
{
	out_x = x;
	out_y = y;
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline void Vector2<T>::SetXY( T newX, T newY )
{
	x = newX;
	y = newY;
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline const Vector2<T> Vector2<T>::operator+( const Vector2<T>& vectorToAdd ) const
{
	return Vector2( x + vectorToAdd.x, y + vectorToAdd.y );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline const Vector2<T> Vector2<T>::operator-( const Vector2<T>& vectorToAdd ) const
{
	return Vector2( x - vectorToAdd.x, y - vectorToAdd.y );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline const Vector2<T> Vector2<T>::operator-( ) const
{
	return Vector2( -x, -y );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
const Vector2<T> Vector2<T>::operator*( const Vector2<T>& vectorToMultiplyComponentwise ) const
{
	return Vector2( x * vectorToMultiplyComponentwise.x,
					y * vectorToMultiplyComponentwise.y );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline const Vector2<T> Vector2<T>::operator*( T scalarToScaleBy ) const
{
	return Vector2( x * scalarToScaleBy, y * scalarToScaleBy );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
inline const Vector2<T> Vector2<T>::operator/( T scalarToScaleBy ) const
{
	ERROR_AND_DIE( "Unexpected Access in Vector2<T>::operator/ !" );
}


//--------------------------------------------------------------------------------------------------------------
template <>
inline const Vector2<int> Vector2<int>::operator/( int scalarToScaleBy ) const
{
	if ( scalarToScaleBy == 0 ) ERROR_AND_DIE( "Vector2 Divided By Scalar Zero" );
	return Vector2( x / scalarToScaleBy, y / scalarToScaleBy );
}


//--------------------------------------------------------------------------------------------------------------
template <>
inline const Vector2<float> Vector2<float>::operator/( float scalarToScaleBy ) const
{
	if ( scalarToScaleBy == 0.f ) ERROR_AND_DIE( "Vector2 Divided By Scalar Zero" );
	return Vector2( x / scalarToScaleBy, y / scalarToScaleBy );
}

//--------------------------------------------------------------------------------------------------------------
template <>
inline const Vector2<double> Vector2<double>::operator/( double scalarToScaleBy ) const
{
	if ( scalarToScaleBy == 0.0 ) ERROR_AND_DIE( "Vector2 Divided By Scalar Zero" );
	return Vector2( x / scalarToScaleBy, y / scalarToScaleBy );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline bool Vector2<T>::operator==( const Vector2<T>& compareTo ) const
{
	return ( x == compareTo.x ) && ( y == compareTo.y );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline bool Vector2<T>::operator!=( const Vector2<T>& compareTo ) const
{
	return !( *this == compareTo );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline void Vector2<T>::operator+=( const Vector2<T>& vectorToAdd )
{
	x += vectorToAdd.x;
	y += vectorToAdd.y;
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline void Vector2<T>::operator-=( const Vector2<T>& vectorToSubtract )
{
	x -= vectorToSubtract.x;
	y -= vectorToSubtract.y;
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline void Vector2<T>::operator*=( const T uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline void Vector2<T>::operator=( const Vector2<T>& vectorToCopy )
{
	x = vectorToCopy.x;
	y = vectorToCopy.y;
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline bool Vector2<T>::operator<( const Vector2<T>& compareTo ) const
{
	if ( this->x < compareTo.x ) return true; //First on x,
	if ( this->x > compareTo.x ) return false;
	return this->y < compareTo.y; //Second ties broken on y.
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline bool Vector2<T>::operator>( const Vector2<T>& compareTo ) const
{
	return !( *this < compareTo );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline float Vector2<T>::CalcFloatLength() const
{
	return static_cast<float>( sqrt( ( x * x ) + ( y * y ) ) );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline float Vector2<T>::CalcFloatLengthSquared() const
{
	return static_cast<float>( ( x * x ) + ( y * y ) );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline double Vector2<T>::CalcDoubleLength() const
{
	return sqrt( ( x * x ) + ( y * y ) );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline double Vector2<T>::CalcDoubleLengthSquared() const
{
	return ( x * x ) + ( y * y );
}


//--------------------------------------------------------------------------------------------------------------
template <>
inline void Vector2<int>::Normalize()
{
	float len = CalcFloatLength();
	if ( len == 0.f ) ERROR_AND_DIE( "Normalizing Vector2 By Length Zero" );

	x = static_cast<int>( x / len );
	y = static_cast<int>( y / len );
}


//--------------------------------------------------------------------------------------------------------------
template <>
inline void Vector2<float>::Normalize()
{
	float len = CalcFloatLength();
	if ( len == 0.f ) ERROR_AND_DIE( "Normalizing Vector2 By Length Zero" );

	x /= len;
	y /= len;
}


//--------------------------------------------------------------------------------------------------------------
template <>
inline void Vector2<double>::Normalize()
{
	double len = CalcDoubleLength();
	if ( len == 0.0 ) ERROR_AND_DIE( "Normalizing Vector2 By Length Zero" );

	x /= len;
	y /= len;
}
