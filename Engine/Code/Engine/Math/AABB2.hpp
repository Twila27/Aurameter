#pragma once


#include "Engine/Math/Vector2.hpp"


template< typename T >
class AABB2
{
public:

	inline AABB2();
	inline AABB2( const Vector2<T>& mins, const Vector2<T>& maxs );
	inline AABB2( T minX, T minY, T maxX, T maxY );
	inline AABB2( const AABB2<T>& sourceToCopy );
	inline T GetWidth() { return maxs.x - mins.x; }
	inline T GetHeight() { return maxs.y - mins.y; }
	inline const Vector2<T> GetCenter() { return Vector2<T>( ( maxs.x + mins.x )/2, ( maxs.y + mins.y )/2 ); }
	inline bool operator==( const AABB2<T>& compareTo ) const;
	inline void AddOffset( const Vector2<T>& offset );
	inline bool ContainsIncludingPerimeter( const Vector2<T>& position );

public:

	Vector2<T> mins;
	Vector2<T> maxs;
};

typedef AABB2<int> AABB2i; //To have a pointer to an AABB2i* ==> have to use AABB2<int>*.
typedef AABB2<float> AABB2f;


//--------------------------------------------------------------------------------------------------------------
// Do-nothing default ctor: because it saves time to leave trash values rather than allocate and initialize.
template < typename T >
inline AABB2<T>::AABB2()
	: mins()
	, maxs()
{
}

template <> //Why is this necessary?
inline AABB2<int>::AABB2()
{
	mins = Vector2<int>();
	maxs = Vector2<int>();
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline AABB2<T>::AABB2( const Vector2<T>& mins, const Vector2<T>& maxs )
	: mins( mins )
	, maxs( maxs )
{
}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline AABB2<T>::AABB2( T minX, T minY, T maxX, T maxY )
	: mins( Vector2<T>( minX, minY ) )
	, maxs( Vector2<T>( maxX, maxY ) )
{
}


//--------------------------------------------------------------------------------------------------------------
template< typename T >
inline AABB2<T>::AABB2( const AABB2<T>& sourceToCopy )
	: mins( sourceToCopy.mins )
	, maxs( sourceToCopy.maxs )
{

}


//--------------------------------------------------------------------------------------------------------------
template < typename T >
inline bool AABB2<T>::operator==( const AABB2<T>& compareTo ) const
{
	return ( mins == compareTo.mins ) && ( maxs == compareTo.maxs );
}


//--------------------------------------------------------------------------------------------------------------
template< typename T >
void AABB2<T>::AddOffset( const Vector2<T>& offset )
{
	mins += offset;
	maxs += offset;
}


//--------------------------------------------------------------------------------------------------------------
template< typename T > 
bool AABB2<T>::ContainsIncludingPerimeter( const Vector2<T>& position )
{
	return ( position.x > mins.x )
		&& ( position.y > mins.y )
		&& ( position.x < maxs.x )
		&& ( position.y < maxs.y );
}