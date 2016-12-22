#pragma once


#include "Engine/EngineCommon.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vector3.hpp"


//DO NOT PROMOTE TO ENGINE: games could for example need to specify view size in tiles versus voxels.
class Camera3D
{
public:
	Camera3D( const Vector3f& worldPosition, const EulerAngles& orientation = EulerAngles::ZERO, bool usesPolarTranslations = false );
	void FixAndClampAngles();
	Vector3f GetForwardXYZ() const;
	Vector3f GetForwardXY() const;
	Vector3f GetLeftXY() const;

public:
	bool m_usesPolarTranslations;
	Vector3f m_worldPosition;
	EulerAngles m_orientation; //Euler angles best solve mouse-look and clamping.
};