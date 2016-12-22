#pragma once


class EulerAngles //Divorced pitch, yaw, roll from x, y, z to make it more basis-agnostic, as x,y,z is divorced from i,j,k.
{
public:

	inline EulerAngles( float pitchDegrees, float yawDegrees, float rollDegrees );

	static const EulerAngles ZERO;

	float m_rollDegrees;
	float m_pitchDegrees;
	float m_yawDegrees;
};


//--------------------------------------------------------------------------------------------------------------
inline EulerAngles::EulerAngles( float pitchDegrees, float yawDegrees, float rollDegrees )
	: m_rollDegrees( pitchDegrees )
	, m_pitchDegrees( yawDegrees )
	, m_yawDegrees( rollDegrees )
{
}
