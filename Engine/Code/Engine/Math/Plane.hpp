#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"


struct Plane
{
	Vector3f initialPosition;
	Vector3f rightPlanarDirection;
	Vector3f upPlanarDirection;
};


inline Vector3f GetPositionOnPlane( const Vector2<float>& position, const void* planeData )
{
	Plane* plane = (Plane*)planeData;

	return plane->initialPosition
		+ ( plane->rightPlanarDirection * position.x )
		+ ( plane->upPlanarDirection * position.y );
}