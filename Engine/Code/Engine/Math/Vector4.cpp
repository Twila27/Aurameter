#include "Engine/Math/Vector4.hpp"
#include "Engine/EngineCommon.hpp"


template <>
STATIC const Vector4<float>  Vector4<float>::ZERO = Vector4( 0.f, 0.f, 0.f, 0.f );
STATIC const Vector4<int>	 Vector4<int>::ZERO = Vector4( 0, 0, 0, 0 );
STATIC const Vector4<double> Vector4<double>::ZERO = Vector4( 0.0, 0.0, 0.0, 0.0 );

template <>
STATIC const Vector4<float>  Vector4<float>::ONE = Vector4( 1.f, 1.f, 1.f, 1.f );
STATIC const Vector4<int>	 Vector4<int>::ONE = Vector4( 1, 1, 1, 1 );
STATIC const Vector4<double> Vector4<double>::ONE = Vector4( 1.0, 1.0, 1.0, 1.0 );

template <>
STATIC const Vector4<float>  Vector4<float>::UNIT_X = Vector4( 1.f, 0.f, 0.f, 0.f );
STATIC const Vector4<float>  Vector4<float>::UNIT_Y = Vector4( 0.f, 1.f, 0.f, 0.f );
STATIC const Vector4<float>  Vector4<float>::UNIT_Z = Vector4( 0.f, 0.f, 1.f, 0.f );
STATIC const Vector4<float>  Vector4<float>::UNIT_W = Vector4( 0.f, 0.f, 0.f, 1.f );
STATIC const Vector4<int>	 Vector4<int>::UNIT_X = Vector4( 1, 0, 0, 0 );
STATIC const Vector4<int>	 Vector4<int>::UNIT_Y = Vector4( 0, 1, 0, 0 );
STATIC const Vector4<int>	 Vector4<int>::UNIT_Z = Vector4( 0, 0, 1, 0 );
STATIC const Vector4<int>	 Vector4<int>::UNIT_W = Vector4( 0, 0, 0, 1 );
STATIC const Vector4<double> Vector4<double>::UNIT_X = Vector4( 1.0, 0.0, 0.0, 0.0 );
STATIC const Vector4<double> Vector4<double>::UNIT_Y = Vector4( 0.0, 1.0, 0.0, 0.0 );
STATIC const Vector4<double> Vector4<double>::UNIT_Z = Vector4( 0.0, 0.0, 1.0, 0.0 );
STATIC const Vector4<double> Vector4<double>::UNIT_W = Vector4( 0.0, 0.0, 0.0, 1.0 );
