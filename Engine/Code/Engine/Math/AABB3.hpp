#pragma once


#include "Engine/Math/Vector3.hpp"


template <typename T>
class AABB3
{
public:

	inline AABB3();
	inline AABB3( const Vector3<T>& mins, const Vector3<T>& maxs );

public:

	Vector3<T> mins;
	Vector3<T> maxs;
};

//--------------------------------------------------------------------------------------------------------------
// Do-nothing default ctor: because it saves time to leave trash values rather than allocate and initialize.
template <typename T>
inline AABB3<T>::AABB3()
	: mins()
	, maxs()
{
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
inline AABB3<T>::AABB3( const Vector3<T>& mins, const Vector3<T>& maxs )
	: mins( mins )
	, maxs( maxs )
{
}

typedef AABB3<int> AABB3i;
typedef AABB3<float> AABB3f;
