#pragma once


#include <math.h>
#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"


template <typename T>
class Vector4
{
public:
	inline Vector4();
	inline Vector4( T initialXYZW );
	inline Vector4( T initialX, T initialY, T initialZ, T initialW );
	inline Vector4( const Vector3<T>& xyz, T w );
	inline Vector4( const Vector4<T>& vectorToCopy );

	inline Vector2<T> xy() const;
	inline Vector3<T> xyz() const;
	inline void GetXYZW( T& out_x, T& out_y, T& out_z, T& out_w ) const;
	inline void SetXYZW( T newX, T newY, T newZ, T newW );

	inline float CalcFloatLength() const;
	inline double CalcDoubleLength() const;
	inline void Normalize();

	inline const Vector4<T> operator+( const Vector4<T>& vectorToAdd ) const;
	inline const Vector4<T> operator-( const Vector4<T>& vectorToSubtract ) const;
	inline const Vector4<T> operator-();
	inline const Vector4<T> operator*( const Vector4<T>& vectorToMultiplyComponentwise ) const;
	inline const Vector4<T> operator*( T scalarToScaleBy ) const;
	inline const Vector4<T> operator/( T scalarToScaleBy ) const;
	inline void operator+=( const Vector4<T>& vectorToAdd );
	inline void operator-=( const Vector4<T>& vectorToSubtract );
	inline void operator*=( const T uniformScale );
	inline void operator=( const Vector4<T>& copyFrom );
	inline bool operator==( const Vector4<T>& compareTo ) const;
	inline bool operator!=( const Vector4<T>& compareTo ) const;

public:

	static const Vector4<T> ZERO;
	static const Vector4<T> ONE;
	static const Vector4<T> UNIT_X;
	static const Vector4<T> UNIT_Y;
	static const Vector4<T> UNIT_Z;
	static const Vector4<T> UNIT_W;

	T x;
	T y;
	T z;
	T w;
};


typedef Vector4<float> Vector4f;
typedef Vector4<int> Vector4i;


//--------------------------------------------------------------------------------------------------------------
// Do-nothing default ctor: because it saves time to leave trash values rather than allocate and initialize.
template < typename T >
inline Vector4<T>::Vector4()
	: x()
	, y()
	, z()
	, w()
{
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline Vector4<T>::Vector4( T initialXYZW ) //Mimicry of GLSL-style extension.
	: x( initialXYZW )
	, y( initialXYZW )
	, z( initialXYZW )
	, w( initialXYZW )
{
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline Vector4<T>::Vector4( T initialX, T initialY, T initialZ, T initialW )
	: x( initialX )
	, y( initialY )
	, z( initialZ )
	, w( initialW )
{
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline Vector4<T>::Vector4( const Vector3<T>& xyz, T w )
	: x( xyz.x )
	, y( xyz.y )
	, z( xyz.z )
	, w( w )
{
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline Vector4<T>::Vector4( const Vector4<T>& vectorToCopy )
	: x( vectorToCopy.x )
	, y( vectorToCopy.y )
	, z( vectorToCopy.z )
	, w( vectorToCopy.w )
{
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline const Vector4<T> Vector4<T>::operator+( const Vector4<T>& vectorToAdd ) const
{
	return Vector4( x + vectorToAdd.x, y + vectorToAdd.y, z + vectorToAdd.z, w + vectorToAdd.w );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline const Vector4<T> Vector4<T>::operator-( const Vector4<T>& vectorToSubtract ) const
{
	return Vector4( x - vectorToSubtract.x, y - vectorToSubtract.y, z - vectorToSubtract.z, w - vectorToSubtract.w );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline const Vector4<T> Vector4<T>::operator-()
{
	x = -x;
	y = -y;
	z = -z;
	w = -w;
	return Vector4( x, y, z, w );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline const Vector4<T> Vector4<T>::operator*( T scalarToScaleBy ) const
{
	return Vector4( x * scalarToScaleBy, y * scalarToScaleBy, z * scalarToScaleBy, w * scalarToScaleBy );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline const Vector4<T> Vector4<T>::operator*( const Vector4<T>& vectorToMultiplyComponentwise ) const
{
	return Vector4( x * vectorToMultiplyComponentwise.x,
					y * vectorToMultiplyComponentwise.y,
					z * vectorToMultiplyComponentwise.z, 
					w * vectorToMultiplyComponentwise.w );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline const Vector4<T> Vector4<T>::operator/( T scalarToScaleBy ) const
{
	ERROR_AND_DIE( "Unexpected Access in Vector4<T>::operator/ !" );
}


//--------------------------------------------------------------------------------------------------------------
template <>
inline const Vector4<int> Vector4<int>::operator/( int scalarToScaleBy ) const
{
	if ( scalarToScaleBy == 0 ) 
		ERROR_AND_DIE( "Vector4i Divided By Scalar Zero" );
	return Vector4( x / scalarToScaleBy, y / scalarToScaleBy, z / scalarToScaleBy, w / scalarToScaleBy );
}


//--------------------------------------------------------------------------------------------------------------
template <>
inline const Vector4<float> Vector4<float>::operator/( float scalarToScaleBy ) const
{
	if ( scalarToScaleBy == 0.f ) 
		ERROR_AND_DIE( "Vector4f Divided By Scalar Zero" );
	return Vector4( x / scalarToScaleBy, y / scalarToScaleBy, z / scalarToScaleBy, w / scalarToScaleBy );
}


//--------------------------------------------------------------------------------------------------------------
template <>
inline const Vector4<double> Vector4<double>::operator/( double scalarToScaleBy ) const
{
	if ( scalarToScaleBy == 0.0 ) 
		ERROR_AND_DIE( "Vector4d Divided By Scalar Zero" );
	return Vector4( x / scalarToScaleBy, y / scalarToScaleBy, z / scalarToScaleBy, w / scalarToScaleBy );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
inline bool Vector4<T>::operator==( const Vector4<T>& compareTo ) const
{
	return ( x == compareTo.x ) && ( y == compareTo.y ) && ( z == compareTo.z ) && ( w == compareTo.w );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
inline bool Vector4<T>::operator!=( const Vector4<T>& compareTo ) const
{
	return !( *this == compareTo );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
inline void Vector4<T>::operator+=( const Vector4<T>& vectorToAdd )
{
	x += vectorToAdd.x;
	y += vectorToAdd.y;
	z += vectorToAdd.z;
	w += vectorToAdd.w;
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
inline void Vector4<T>::operator-=( const Vector4<T>& vectorToSubtract )
{
	x -= vectorToSubtract.x;
	y -= vectorToSubtract.y;
	z -= vectorToSubtract.z;
	w -= vectorToSubtract.w;
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
inline void Vector4<T>::operator*=( const T uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
	w *= uniformScale;
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
inline void Vector4<T>::operator=( const Vector4<T>& vectorToCopy )
{
	x = vectorToCopy.x;
	y = vectorToCopy.y;
	z = vectorToCopy.z;
	w = vectorToCopy.w;
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline float Vector4<T>::CalcFloatLength() const
{
	return static_cast<float>( sqrt( ( x * x ) + ( y * y ) + ( z * z ) + ( w * w ) ) );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline double Vector4<T>::CalcDoubleLength() const
{
	return sqrt( ( x * x ) + ( y * y ) + ( z * z ) + ( w * w ) );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline Vector2<T> Vector4<T>::xy() const
{
	return Vector2<T>( x, y );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline Vector3<T> Vector4<T>::xyz() const
{
	return Vector3<T>( x, y, z );
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline void Vector4<T>::GetXYZW( T& out_x, T& out_y, T& out_z, T& out_w ) const
{
	out_x = x;
	out_y = y;
	out_z = z;
	out_w = w;
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline void Vector4<T>::SetXYZW( T newX, T newY, T newZ, T newW )
{
	x = newX;
	y = newY;
	z = newZ;
	w = newW;
}


//--------------------------------------------------------------------------------------------------------------
template <>
inline void Vector4<int>::Normalize()
{
	float len = CalcFloatLength();
	if ( len == 0.f ) ERROR_AND_DIE( "Normalizing Vector4i By Length Zero" );

	x = static_cast<int>( x / len );
	y = static_cast<int>( y / len );
	z = static_cast<int>( z / len );
	w = static_cast<int>( w / len );
}


//--------------------------------------------------------------------------------------------------------------
template <>
inline void Vector4<float>::Normalize()
{
	float len = CalcFloatLength();
	if ( len == 0.f ) ERROR_AND_DIE( "Normalizing Vector4f By Length Zero" );

	x /= len;
	y /= len;
	z /= len;
	w /= len;
}


//--------------------------------------------------------------------------------------------------------------
template <>
inline void Vector4<double>::Normalize()
{
	double len = CalcDoubleLength();
	if ( len == 0.0 ) 
		ERROR_AND_DIE( "Normalizing Vector4d By Length Zero" );

	x /= len;
	y /= len;
	z /= len;
	w /= len;
}
