#include "Engine/Math/Camera3D.hpp"
#include "Engine/Math/MathUtils.hpp"


//--------------------------------------------------------------------------------------------------------------
Camera3D::Camera3D( const Vector3f& worldPosition, const EulerAngles& orientation /*= EulerAngles::ZERO*/, bool usesPolarTranslations /*= false*/ )
	: m_worldPosition( worldPosition )
	, m_orientation( orientation )
	, m_usesPolarTranslations( usesPolarTranslations )
{
}


//--------------------------------------------------------------------------------------------------------------
void Camera3D::FixAndClampAngles() //See TheGame::UpdateFromMouse() on basis details.
{
	//Basis-DEPENDENT, because choice of basis defines ijk, which defines which rotation dimension is unclamped:
	m_orientation.m_rollDegrees = ClampFloat( m_orientation.m_rollDegrees, -89.f, 89.f );
	m_orientation.m_pitchDegrees = ClampFloat( m_orientation.m_pitchDegrees, -89.f, 89.f );
	m_orientation.m_yawDegrees = WrapNumberWithinCircularRange( m_orientation.m_yawDegrees, 0.f, 360.f );
}


//--------------------------------------------------------------------------------------------------------------
Vector3f Camera3D::GetForwardXYZ() const //Same heading cos(j-vector world-up), sin(j-vector world-up), but scaled by cos(i-vector world-right) as in polar form.
{
	float theta = m_orientation.m_yawDegrees;
	float phi = m_orientation.m_pitchDegrees;

	float radiusScaleAlongXY = CosDegrees( phi );
	return Vector3f( radiusScaleAlongXY * CosDegrees( theta ), radiusScaleAlongXY * SinDegrees( theta ), -SinDegrees( phi ) );
}


//--------------------------------------------------------------------------------------------------------------
Vector3f Camera3D::GetForwardXY() const //Just a 2D heading using j_vector (world up) with zero z-component.
{
	float cosine = CosDegrees( m_orientation.m_yawDegrees );
	float sine = SinDegrees( m_orientation.m_yawDegrees );
	return Vector3f( cosine, sine, 0.f );
}


//--------------------------------------------------------------------------------------------------------------
Vector3f Camera3D::GetLeftXY() const //Just a 2D heading with zero z-component, negated by our backward y-axis.
{
	Vector3f forwardXY = GetForwardXY();
	return Vector3f( -forwardXY.y, forwardXY.x, 0.f );
}
