#pragma once


template <typename T>
class PolarCoords
{
public:

	inline PolarCoords();
	inline PolarCoords( T initialX, T initialY );
	inline PolarCoords( const PolarCoords<T>& coordsToCopy );


public:

	T radius;
	T thetaRadians;
};


//--------------------------------------------------------------------------------------------------------------
// Do-nothing ctor: because it's faster to leave trash values than to allocate and initialize.
template <typename T>
inline PolarCoords<T>::PolarCoords( )
	: radius()
	, thetaRadians()
{
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
inline PolarCoords<T>::PolarCoords( T initialRadius, T initialThetaRadians )
	: radius( initialRadius )
	, thetaRadians( initialThetaRadians )
{
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
inline PolarCoords<T>::PolarCoords( const PolarCoords<T>& coordsToCopy )
	: radius( coordsToCopy.radius )
	, thetaRadians( coordsToCopy.thetaRadians )
{
}
