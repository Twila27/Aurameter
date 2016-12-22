#pragma once


#include "Engine/EngineCommon.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/AABB2.hpp"


class Camera2D
{
public:
	Camera2D( const Vector2f& worldPosition, const float orientationDegrees = 0.f, bool usesPolarTranslations = false );
	Vector2f GetForwardXY() const;
	Vector2f GetLeftXY() const;
	Matrix4x4f GetViewTransform() const;
	void SatisfyConstraints();

public:
	bool m_usesPolarTranslations;
	Vector2f m_worldPosition;
	float m_orientationDegrees;
	float m_speedScale;

	bool m_usesScrollingLimits;
	AABB2f m_worldScrollingLimits;

private:
	void UpdateViewTransform();
	Matrix4x4f m_currentViewTransform;
};
