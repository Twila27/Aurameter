#include "Engine/Math/Vector2.hpp"
#include "Engine/EngineCommon.hpp"


template <>
STATIC const Vector2<float> Vector2<float>::ZERO = Vector2( 0.f, 0.f );
STATIC const Vector2<int> Vector2<int>::ZERO = Vector2( 0, 0 );
STATIC const Vector2<double> Vector2<double>::ZERO = Vector2( 0.0, 0.0 );

template <>
STATIC const Vector2<float> Vector2<float>::ONE = Vector2( 1.f, 1.f );
STATIC const Vector2<int> Vector2<int>::ONE = Vector2( 1, 1 );
STATIC const Vector2<double> Vector2<double>::ONE = Vector2( 1, 1 );

template <>
STATIC const Vector2<float> Vector2<float>::UNIT_X = Vector2( 1.f, 0.f );
STATIC const Vector2<float> Vector2<float>::UNIT_Y = Vector2( 0.f, 1.f );
STATIC const Vector2<int> Vector2<int>::UNIT_X = Vector2( 1, 0 );
STATIC const Vector2<int> Vector2<int>::UNIT_Y = Vector2( 0, 1 );
STATIC const Vector2<double> Vector2<double>::UNIT_X = Vector2( 1.0, 0.0 );
STATIC const Vector2<double> Vector2<double>::UNIT_Y = Vector2( 0.0, 1.0 );
