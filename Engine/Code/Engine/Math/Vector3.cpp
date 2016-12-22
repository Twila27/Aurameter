#include "Engine/Math/Vector3.hpp"
#include "Engine/EngineCommon.hpp"


template <>
STATIC const Vector3<float>  Vector3<float>::ZERO =  Vector3( 0.f, 0.f, 0.f );
STATIC const Vector3<int>	 Vector3<int>::ZERO =	 Vector3( 0, 0, 0 );
STATIC const Vector3<double> Vector3<double>::ZERO = Vector3( 0.0, 0.0, 0.0 );

template <>
STATIC const Vector3<float>  Vector3<float>::ONE =  Vector3( 1.f, 1.f, 1.f );
STATIC const Vector3<int>	 Vector3<int>::ONE =	Vector3( 1, 1, 1 );
STATIC const Vector3<double> Vector3<double>::ONE = Vector3( 1.0, 1.0, 1.0 );

template <>
STATIC const Vector3<float>  Vector3<float>::UNIT_X = Vector3( 1.f, 0.f, 0.f );
STATIC const Vector3<float>  Vector3<float>::UNIT_Y = Vector3( 0.f, 1.f, 0.f );
STATIC const Vector3<float>  Vector3<float>::UNIT_Z = Vector3( 0.f, 0.f, 1.f );
STATIC const Vector3<int>	 Vector3<int>::UNIT_X = Vector3( 1, 0, 0 );
STATIC const Vector3<int>	 Vector3<int>::UNIT_Y = Vector3( 0, 1, 0 );
STATIC const Vector3<int>	 Vector3<int>::UNIT_Z = Vector3( 0, 0, 1 );
STATIC const Vector3<double> Vector3<double>::UNIT_X = Vector3( 1.0, 0.0, 0.0 );
STATIC const Vector3<double> Vector3<double>::UNIT_Y = Vector3( 0.0, 1.0, 0.0 );
STATIC const Vector3<double> Vector3<double>::UNIT_Z = Vector3( 0.0, 0.0, 1.0 );
