#pragma once


#include <math.h>
#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/Math/Vector2.hpp"


template <typename T>
class Vector3
{
public:
	inline Vector3();
	inline Vector3( T initialXYZ );
	inline Vector3( T initialX, T initialY, T initialZ );
	inline Vector3( const Vector3<T>& vectorToCopy );

	inline Vector2<T> xy() const;
	inline Vector3<T> xyz() const { return Vector3f( x, y, z ); }
	inline Vector3<T> xzy() const { return Vector3f( x, z, y ); }
	inline Vector3<T> yxz() const { return Vector3f( y, x, z ); }
	inline Vector3<T> yzx() const { return Vector3f( y, z, x ); }
	inline Vector3<T> zxy() const { return Vector3f( z, x, z ); }
	inline Vector3<T> zyx() const { return Vector3f( z, y, x ); }
	inline void GetXYZ( T& out_x, T& out_y, T& out_z ) const;
	inline void SetXYZ( T newX, T newY, T newZ );

	inline float CalcFloatLength() const;
	inline double CalcDoubleLength() const;
	inline void Normalize();

	inline const Vector3<T> operator+( const Vector3<T>& vectorToAdd ) const;
	inline const Vector3<T> operator-( const Vector3<T>& vectorToSubtract ) const;
	inline const Vector3<T> operator-();
	inline const Vector3<T> operator*( T scalarToScaleBy ) const;
	inline const Vector3<T> operator/( T scalarToScaleBy ) const;
	inline void operator+=( const Vector3<T>& vectorToAdd );
	inline void operator-=( const Vector3<T>& vectorToSubtract );
	inline void operator*=( const T uniformScale );
	inline void operator=( const Vector3<T>& copyFrom );
	inline bool operator==( const Vector3<T>& compareTo ) const;
	inline bool operator!=( const Vector3<T>& compareTo ) const;

public: //Because Vector3f is virtually a primitive.

	static const Vector3<T> ZERO;
	static const Vector3<T> ONE;
	static const Vector3<T> UNIT_X;
	static const Vector3<T> UNIT_Y;
	static const Vector3<T> UNIT_Z;

	T x;
	T y;
	T z;
};


typedef Vector3<float> Vector3f;
typedef Vector3<int> Vector3i;


//--------------------------------------------------------------------------------------------------------------
// Do-nothing default ctor: because it saves time to leave trash values rather than allocate and initialize.
template < typename T >
inline Vector3<T>::Vector3()
	: x()
	, y()
	, z()
{
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline Vector3<T>::Vector3( T initialXYZ ) //Mimicry of GLSL-style extension.
	: x( initialXYZ )
	, y( initialXYZ )
	, z( initialXYZ )
{
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline Vector3<T>::Vector3( T initialX, T initialY, T initialZ )
	: x( initialX )
	, y( initialY )
	, z( initialZ )
{
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline Vector3<T>::Vector3( const Vector3<T>& vectorToCopy )
	: x( vectorToCopy.x )
	, y( vectorToCopy.y )
	, z( vectorToCopy.z )
{
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline const Vector3<T> Vector3<T>::operator+( const Vector3<T>& vectorToAdd ) const
{
	return Vector3( x + vectorToAdd.x, y + vectorToAdd.y, z + vectorToAdd.z );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline const Vector3<T> Vector3<T>::operator-( const Vector3<T>& vectorToSubtract ) const
{
	return Vector3( x - vectorToSubtract.x, y - vectorToSubtract.y, z - vectorToSubtract.z );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline const Vector3<T> Vector3<T>::operator-( )
{
	x = -x;
	y = -y;
	z = -z;
	return Vector3( x, y, z );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline const Vector3<T> Vector3<T>::operator*( T scalarToScaleBy ) const
{
	return Vector3( x * scalarToScaleBy, y * scalarToScaleBy, z * scalarToScaleBy );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline const Vector3<T> Vector3<T>::operator/( T scalarToScaleBy ) const
{
	ERROR_AND_DIE( "Unexpected Access in Vector3<T>::operator/ !" );
}


//--------------------------------------------------------------------------------------------------------------
template <>
inline const Vector3<int> Vector3<int>::operator/( int scalarToScaleBy ) const
{
	if ( scalarToScaleBy == 0 ) 
		ERROR_AND_DIE( "Vector3i Divided By Scalar Zero" );
	return Vector3( x / scalarToScaleBy, y / scalarToScaleBy, z / scalarToScaleBy );
}


//--------------------------------------------------------------------------------------------------------------
template <>
inline const Vector3<float> Vector3<float>::operator/( float scalarToScaleBy ) const
{
	if ( scalarToScaleBy == 0.f ) 
		ERROR_AND_DIE( "Vector3f Divided By Scalar Zero" );
	return Vector3( x / scalarToScaleBy, y / scalarToScaleBy, z / scalarToScaleBy );
}


//--------------------------------------------------------------------------------------------------------------
template <>
inline const Vector3<double> Vector3<double>::operator/( double scalarToScaleBy ) const
{
	if ( scalarToScaleBy == 0.0 ) 
		ERROR_AND_DIE( "Vector3d Divided By Scalar Zero" );
	return Vector3( x / scalarToScaleBy, y / scalarToScaleBy, z / scalarToScaleBy );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline bool Vector3<T>::operator==( const Vector3<T>& compareTo ) const
{
	return ( x == compareTo.x ) && ( y == compareTo.y ) && ( z == compareTo.z );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline bool Vector3<T>::operator!=( const Vector3<T>& compareTo ) const
{
	return !( *this == compareTo );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline void Vector3<T>::operator+=( const Vector3<T>& vectorToAdd )
{
	x += vectorToAdd.x;
	y += vectorToAdd.y;
	z += vectorToAdd.z;
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline void Vector3<T>::operator-=( const Vector3<T>& vectorToSubtract )
{
	x -= vectorToSubtract.x;
	y -= vectorToSubtract.y;
	z -= vectorToSubtract.z;
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline void Vector3<T>::operator*=( const T uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline void Vector3<T>::operator=( const Vector3<T>& vectorToCopy )
{
	x = vectorToCopy.x;
	y = vectorToCopy.y;
	z = vectorToCopy.z;
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline float Vector3<T>::CalcFloatLength() const
{
	return static_cast<float>( sqrt( ( x * x ) + ( y * y ) + ( z * z ) ) );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline double Vector3<T>::CalcDoubleLength() const
{
	return sqrt( ( x * x ) + ( y * y ) + ( z * z ) );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline Vector2<T> Vector3<T>::xy() const
{
	return Vector2<T>( x, y );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline void Vector3<T>::GetXYZ( T& out_x, T& out_y, T& out_z ) const
{
	out_x = x;
	out_y = y;
	out_z = z;
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline void Vector3<T>::SetXYZ( T newX, T newY, T newZ )
{
	x = newX;
	y = newY;
	z = newZ;
}


//--------------------------------------------------------------------------------------------------------------
template <>
inline void Vector3<int>::Normalize()
{
	float len = CalcFloatLength();
	if ( len == 0.f ) 
		ERROR_AND_DIE( "Normalizing Vector3i By Length Zero" );

	x = static_cast<int>( x / len );
	y = static_cast<int>( y / len );
	z = static_cast<int>( z / len );
}


//--------------------------------------------------------------------------------------------------------------
template <>
inline void Vector3<float>::Normalize()
{
	float len = CalcFloatLength();
	if ( len == 0.f ) 
		ERROR_AND_DIE( "Normalizing Vector3f By Length Zero" );

	x /= len;
	y /= len;
	z /= len;
}


//--------------------------------------------------------------------------------------------------------------
template <>
inline void Vector3<double>::Normalize()
{
	double len = CalcDoubleLength();
	if ( len == 0.0 ) 
		ERROR_AND_DIE( "Normalizing Vector3d By Length Zero" );

	x /= len;
	y /= len;
	z /= len;
}
