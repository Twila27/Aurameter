#include "Engine/Math/Camera2D.hpp"
#include "Engine/Math/MathUtils.hpp"


//--------------------------------------------------------------------------------------------------------------
Camera2D::Camera2D( const Vector2f& worldPosition, float orientationDegrees /*= 0.f*/, bool usesPolarTranslations /*= false*/ )
	: m_worldPosition( worldPosition )
	, m_orientationDegrees( orientationDegrees )
	, m_usesPolarTranslations( usesPolarTranslations )
	, m_usesScrollingLimits( false )
	, m_currentViewTransform( COLUMN_MAJOR )
	, m_speedScale( 8.f )
{
}


//--------------------------------------------------------------------------------------------------------------
Vector2f Camera2D::GetForwardXY() const //Just a 2D heading using j_vector (world up) with zero z-component.
{
	float cosine = CosDegrees( m_orientationDegrees );
	float sine = SinDegrees( m_orientationDegrees );
	return Vector2f( cosine, sine );
}


//--------------------------------------------------------------------------------------------------------------
Vector2f Camera2D::GetLeftXY() const //Just a 2D heading with zero z-component, negated by our backward y-axis.
{
	Vector2f forwardXY = GetForwardXY();
	return Vector2f( -forwardXY.y, forwardXY.x ); //Need to verify whether this still holds!
}


//--------------------------------------------------------------------------------------------------------------
Matrix4x4f Camera2D::GetViewTransform() const
{
	return m_currentViewTransform;
}


//--------------------------------------------------------------------------------------------------------------
void Camera2D::UpdateViewTransform()
{
	//In the scheme of Model * View * Projection, Model takes a vertex from object/model/sprite space to world space.
	//Then it's on the View transform to get from world to camera space.
	//Together, the camera rotation and translation describe the camera relative to the world, so we need to invert it before returning.
	//Because rotation matrices are orthogonal, their inverted form is just their transpose.
	//Similarly, translation matrices when inverted are just their offsets now negated.
	//However, the invert also flips their order of multiplication from the normal v*SRT to then be (-T)*(R^Transpose).

	m_currentViewTransform.ClearToRotationMatrix( -m_orientationDegrees, m_currentViewTransform.GetOrdering() );

	m_currentViewTransform.SetTranslation( Vector3f( m_worldPosition.x, m_worldPosition.y, 0.f ), m_currentViewTransform.GetOrdering() );

	m_currentViewTransform.GetInverseAssumingOrthonormality( m_currentViewTransform );
}


//--------------------------------------------------------------------------------------------------------------
void Camera2D::SatisfyConstraints()
{
	m_orientationDegrees = WrapNumberWithinCircularRange( m_orientationDegrees, 0.f, 360.f );

	if ( !m_usesScrollingLimits )
	{
		UpdateViewTransform();
		return;
	}
	else 
	{
		if ( m_worldPosition.x < m_worldScrollingLimits.mins.x )
			m_worldPosition.x = m_worldScrollingLimits.mins.x;

		if ( m_worldPosition.x > m_worldScrollingLimits.maxs.x )
			m_worldPosition.x = m_worldScrollingLimits.maxs.x;

		if ( m_worldPosition.y < m_worldScrollingLimits.mins.y )
			m_worldPosition.y = m_worldScrollingLimits.mins.y;

		if ( m_worldPosition.y > m_worldScrollingLimits.maxs.y )
			m_worldPosition.y = m_worldScrollingLimits.maxs.y;

		UpdateViewTransform();
		return;
	}
}
