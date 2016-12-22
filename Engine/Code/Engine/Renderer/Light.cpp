#include "Engine/Renderer/Light.hpp"
#include "Engine/EngineCommon.hpp"
#include "Engine/Core/TheConsole.hpp"
#include "Engine/Core/Command.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Renderer/DebugRenderCommand.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC LightRegistryVector		Light::s_lightRegistry;
STATIC const float				Light::s_renderRadius = .1f;

//--------------------------------------------------------------------------------------------------------------
void ToggleLight( Command& args )
{
	int lightIndex = 0;
	bool success = args.GetNextInt( &lightIndex, -1 );
	if ( !success )
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: ToggleLight <0-15>" );
		return;
	}

	Light::ToggleLight( lightIndex );
}


//--------------------------------------------------------------------------------------------------------------
void ToggleLights( Command& /*args*/ )
{
	static bool currentToggleValue = false;
	
	for ( Light* light : *Light::GetRegistry() )
		light->SetIsLightOn( currentToggleValue );

	currentToggleValue = !currentToggleValue;
}


//--------------------------------------------------------------------------------------------------------------
STATIC Light* Light::CreateLight( LightType type, const Vector3f& position, const Vector3f& direction, const Rgba& color,
								float minLightDistRadius /*= 0.f*/, float maxLightDistRadius /*= INFINITY*/,
								float powerAtMinDist /*= 0.f*/, float powerAtMaxDist /*= 0.f*/,
								float innerAngleCosine /*= 0.f*/, float outerAngleCosine /*= 0.f*/,
								float powerInsideInnerAngle /*= 0.f*/, float powerOutsideOuterAngle /*= 0.f*/ )
{
	ASSERT_OR_DIE( ( s_lightRegistry.size() + 1 ) <= LIGHTS_IN_ENGINE_MAX, "Exceeded MAX_LIGHTS in CreateOrGetLight!" );
	if ( s_lightRegistry.size() + 1 > LIGHTS_IN_ENGINE_MAX )
		return nullptr;

	s_lightRegistry.push_back( new Light(type, position, direction, color, minLightDistRadius, maxLightDistRadius, 
		powerAtMinDist, powerAtMaxDist, 
		innerAngleCosine, outerAngleCosine, 
		powerInsideInnerAngle, powerOutsideOuterAngle) );

	return s_lightRegistry.back();
}


//--------------------------------------------------------------------------------------------------------------
STATIC void Light::DeleteLights()
{
	for ( Light* light : s_lightRegistry )
	{
		if ( light != nullptr )
		{
			delete light;
			light = nullptr;
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
STATIC void Light::GetLightPositions( Vector3f* out )
{
	for ( unsigned int lightIndex = 0; lightIndex < s_lightRegistry.size(); lightIndex++ )
		out[ lightIndex ] = s_lightRegistry[ lightIndex ]->m_position;
}


//--------------------------------------------------------------------------------------------------------------
STATIC void Light::GetLightDirections( Vector3f* out )
{
	for ( unsigned int lightIndex = 0; lightIndex < s_lightRegistry.size(); lightIndex++ )
		out[ lightIndex ] = s_lightRegistry[ lightIndex ]->m_direction;
}


//--------------------------------------------------------------------------------------------------------------
STATIC void Light::GetLightColors( Rgba* out )
{
	for ( unsigned int lightIndex = 0; lightIndex < s_lightRegistry.size(); lightIndex++ )
		out[ lightIndex ] = s_lightRegistry[ lightIndex ]->m_color;
}


//--------------------------------------------------------------------------------------------------------------
STATIC void Light::GetLightIsOnFlags( int* out )
{
	for ( unsigned int lightIndex = 0; lightIndex < s_lightRegistry.size(); lightIndex++ )
		out[ lightIndex ] = s_lightRegistry[ lightIndex ]->m_isLightOn;
}


//--------------------------------------------------------------------------------------------------------------
STATIC void Light::GetLightIsDirectionalFlags( int* out )
{
	for ( unsigned int lightIndex = 0; lightIndex < s_lightRegistry.size(); lightIndex++ )
		out[ lightIndex ] = s_lightRegistry[ lightIndex ]->m_lerpTrickIsDirectional;
}


//--------------------------------------------------------------------------------------------------------------
STATIC void Light::GetLightMinDistances( float* out )
{
	for ( unsigned int lightIndex = 0; lightIndex < s_lightRegistry.size(); lightIndex++ )
		out[ lightIndex ] = s_lightRegistry[ lightIndex ]->m_minLightDistance;
}


//--------------------------------------------------------------------------------------------------------------
STATIC void Light::GetLightMaxDistances( float* out )
{
	for ( unsigned int lightIndex = 0; lightIndex < s_lightRegistry.size(); lightIndex++ )
		out[ lightIndex ] = s_lightRegistry[ lightIndex ]->m_maxLightDistance;
}


//--------------------------------------------------------------------------------------------------------------
STATIC void Light::GetLightPowersAtMinDists( float* out )
{
	for ( unsigned int lightIndex = 0; lightIndex < s_lightRegistry.size(); lightIndex++ )
		out[ lightIndex ] = s_lightRegistry[ lightIndex ]->m_isLightOn ? s_lightRegistry[ lightIndex ]->m_powerAtMinDistance : 0.0f;
}


//--------------------------------------------------------------------------------------------------------------
STATIC void Light::GetLightPowersAtMaxDists( float* out )
{
	for ( unsigned int lightIndex = 0; lightIndex < s_lightRegistry.size(); lightIndex++ )
		out[ lightIndex ] = s_lightRegistry[ lightIndex ]->m_isLightOn ? s_lightRegistry[ lightIndex ]->m_powerAtMaxDistance : 0.0f;
}


//--------------------------------------------------------------------------------------------------------------
STATIC void Light::GetLightInnerAngleCosines( float* out )
{
	for ( unsigned int lightIndex = 0; lightIndex < s_lightRegistry.size(); lightIndex++ )
		out[ lightIndex ] = s_lightRegistry[ lightIndex ]->m_innerAngleCosine;
}


//--------------------------------------------------------------------------------------------------------------
STATIC void Light::GetLightOuterAngleCosines( float* out )
{
	for ( unsigned int lightIndex = 0; lightIndex < s_lightRegistry.size(); lightIndex++ )
		out[ lightIndex ] = s_lightRegistry[ lightIndex ]->m_outerAngleCosine;
}


//--------------------------------------------------------------------------------------------------------------
STATIC void Light::GetLightPowersInsideInnerAngles( float* out )
{
	for ( unsigned int lightIndex = 0; lightIndex < s_lightRegistry.size(); lightIndex++ )
		out[ lightIndex ] = s_lightRegistry[ lightIndex ]->m_isLightOn ? s_lightRegistry[ lightIndex ]->m_powerInsideInnerAngle : 0.0f;
}


//--------------------------------------------------------------------------------------------------------------
STATIC void Light::GetLightPowersOutsideOuterAngles( float* out )
{
	for ( unsigned int lightIndex = 0; lightIndex < s_lightRegistry.size(); lightIndex++ )
		out[ lightIndex ] = s_lightRegistry[ lightIndex ]->m_isLightOn ? s_lightRegistry[ lightIndex ]->m_powerOutsideOuterAngle : 0.0f;
}


//--------------------------------------------------------------------------------------------------------------
void Light::ToggleLight( unsigned int lightIndex )
{
	if ( lightIndex >= s_lightRegistry.size() )
	{
		g_theConsole->ShowConsole();
		g_theConsole->Printf( "Not enough lights to access lightIndex: " + lightIndex );
		return;
	}
	s_lightRegistry[ lightIndex ]->m_isLightOn = !s_lightRegistry[ lightIndex ]->m_isLightOn;
}


//--------------------------------------------------------------------------------------------------------------
Light::Light( LightType type, const Vector3f& position, const Vector3f& direction, const Rgba& color,
			  float minLightDistRadius /*= 0.f*/, float maxLightDistRadius /*= INFINITY*/,
			  float powerAtMinDist /*= 0.f*/, float powerAtMaxDist /*= 0.f*/,
			  float innerAngleCosine /*= 0.f*/, float outerAngleCosine /*= 0.f*/,
			  float powerInsideInnerAngle /*= 0.f*/, float powerOutsideOuterAngle /*= 0.f*/ ) //maxLightDistRadius == INFINITY for global / no falloff.
	: m_isLightOn(true) //Initially always off.
{
	switch ( type )
	{
	case POINT_LIGHT:
		MakePointLight( position, direction, color, minLightDistRadius, maxLightDistRadius );
		break;
	case DIRECTIONAL_LIGHT:
		MakeDirectionalLight( position, direction, color );
		break;
	case SPOT_LIGHT:
		MakeSpotLight( position, direction, color, minLightDistRadius, maxLightDistRadius, 
					   powerAtMinDist, powerAtMaxDist, 
					   innerAngleCosine, outerAngleCosine, 
					   powerInsideInnerAngle, powerOutsideOuterAngle );
		break;
	}
}


//--------------------------------------------------------------------------------------------------------------
void Light::MakePointLight( const Vector3f& position, const Vector3f& direction, const Rgba& color, 
	float minLightDistRadius, float maxLightDistRadius = INFINITY )
{
	m_position = position;
	m_direction = direction;
	m_color = color;
	m_minLightDistance = minLightDistRadius;
	m_maxLightDistance = maxLightDistRadius; //Make INF for global!
	m_powerAtMinDistance = 1.f;
	m_powerAtMaxDistance = 0.f;
	m_innerAngleCosine = 1.f;
	m_outerAngleCosine = -1.f;
	m_powerInsideInnerAngle = 1.f;
	m_powerOutsideOuterAngle = 1.f;
	m_lerpTrickIsDirectional = 0;
}


//--------------------------------------------------------------------------------------------------------------
void Light::MakeSpotLight( const Vector3f& position, const Vector3f& direction, const Rgba& color, 
	float minLightDistRadius, float maxLightDistRadius,
	float powerAtMinDist, float powerAtMaxDist,
	float innerAngleCosine, float outsideAngleCosine,
	float powerInsideInnerAngle, float powerOutsideOuterAngle )
{
	m_position = position;
	m_direction = direction;
	m_color = color;
	m_minLightDistance = minLightDistRadius;
	m_maxLightDistance = maxLightDistRadius;
	m_powerAtMinDistance = powerAtMinDist;
	m_powerAtMaxDistance = powerAtMaxDist;
	m_innerAngleCosine = innerAngleCosine;
	m_outerAngleCosine = outsideAngleCosine;
	m_powerInsideInnerAngle = powerInsideInnerAngle;
	m_powerOutsideOuterAngle = powerOutsideOuterAngle;
	m_lerpTrickIsDirectional = 0;
}


//--------------------------------------------------------------------------------------------------------------
void Light::MakeDirectionalLight( const Vector3f& position, const Vector3f& direction, const Rgba& color )
{
	m_position = position;
	m_direction = direction;
	m_color = color;
	m_minLightDistance = 0.f;
	m_maxLightDistance = INFINITY;
	m_powerAtMinDistance = 1.f;
	m_powerAtMaxDistance = 1.f;
	m_innerAngleCosine = 1.f;
	m_outerAngleCosine = -1.f;
	m_powerInsideInnerAngle = 1.f;
	m_powerOutsideOuterAngle = 1.f;
	m_lerpTrickIsDirectional = 1;
}


//--------------------------------------------------------------------------------------------------------------
void Light::Render() const
{
	if ( !GetIsLightOn() )
		return;

	Rgba lightColor = GetColor();

	if ( lightColor == Rgba::BLACK )
		return; //Not rendering "disabled" lights that aren't contributing to the scene!

	g_theDebugRenderCommands->push_back( new DebugRenderCommandSphere( GetPosition(), s_renderRadius, 0.f, DEPTH_TEST_ON, lightColor, 1.f ) );
}
