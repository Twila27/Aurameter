#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/Rgba.hpp"
#include "Engine/EngineCommon.hpp"
#include "Engine/Memory/UntrackedAllocator.hpp"
#include <vector>


//-----------------------------------------------------------------------------
enum LightType { POINT_LIGHT, DIRECTIONAL_LIGHT, SPOT_LIGHT };


//-----------------------------------------------------------------------------
class Light;
typedef std::vector<Light*, UntrackedAllocator<Light*> > LightRegistryVector;


//-----------------------------------------------------------------------------
class Command;
void ToggleLight( Command& args );
void ToggleLights( Command& args );


//-----------------------------------------------------------------------------
//For global, make maxLightDistRadius := INFINITY, cf. MakeDirectionalLight config.
class Light
{
public:

	static Light* CreateLight( LightType type, const Vector3f& position, const Vector3f& direction, const Rgba& color,
		float minLightDistRadius = 0.f, float maxLightDistRadius = INFINITY,
		float powerAtMinDist = 0.f, float powerAtMaxDist = 0.f,
		float innerAngleCosine = 0.f, float outerAngleCosine = 0.f,
		float powerInsideInnerAngle = 0.f, float powerOutsideOuterAngle = 0.f );
	static void DeleteLights();

	static Light* GetLight( unsigned int lightIndex ) { return ( lightIndex < s_lightRegistry.size() ) ? s_lightRegistry[ lightIndex ] : nullptr; }
	static const LightRegistryVector* GetRegistry() { return &s_lightRegistry; }

	static void GetLightPositions( Vector3f* out );
	static void GetLightDirections( Vector3f* out );
	static void GetLightColors( Rgba* out );
	static void GetLightIsOnFlags( int* out );
	static void GetLightIsDirectionalFlags( int* out );
	static void GetLightMinDistances( float* out );
	static void GetLightMaxDistances( float* out );
	static void GetLightPowersAtMinDists( float* out );
	static void GetLightPowersAtMaxDists( float* out );
	static void GetLightInnerAngleCosines( float* out );
	static void GetLightOuterAngleCosines( float* out );
	static void GetLightPowersInsideInnerAngles( float* out );
	static void GetLightPowersOutsideOuterAngles( float* out );

	static void ToggleLight( unsigned int lightIndex );

	void Translate( const Vector3f& position ) { m_position += position; }
	void SetPosition( const Vector3f& position ) { m_position = position; }
	void SetColor( const Rgba& color ) { m_color = color; }
	void SetDirection( const Vector3f& direction ) { m_direction = direction; }
	void SetMinDistance( float newVal ) { m_minLightDistance = newVal; }
	void SetMaxDistance( float newVal ) { m_maxLightDistance = newVal; }
	void SetPowerAtMinDistance( float newVal ) { m_powerAtMinDistance = newVal; }
	void SetPowerAtMaxDistance( float newVal ) { m_powerAtMaxDistance = newVal; }
	void SetInnerAngleCosine( float newVal ) { m_innerAngleCosine = newVal; }
	void SetOuterAngleCosine( float newVal ) { m_outerAngleCosine = newVal; }
	void SetPowerInsideInnerAngleCosine( float newVal ) { m_powerInsideInnerAngle = newVal; }
	void SetPowerOutsideOuterAngleCosine( float newVal ) { m_powerOutsideOuterAngle = newVal; }
	void SetIsDirectional( int newVal ) { m_lerpTrickIsDirectional = newVal; }
	void SetIsLightOn( bool newVal ) { m_isLightOn = newVal; }
	bool GetIsLightOn() const { return m_isLightOn; }
	Rgba GetColor() const { return m_color; }
	Vector3f GetPosition() const { return m_position; }

	void Render() const;


private:

	Light( LightType type, const Vector3f& position, const Vector3f& direction, const Rgba& color,
		   float minLightDistRadius = 0.f, float maxLightDistRadius = INFINITY,
		   float powerAtMinDist = 0.f, float powerAtMaxDist = 0.f, 
		   float innerAngleCosine = 0.f, float outerAngleCosine = 0.f, 
		   float powerInsideInnerAngle = 0.f, float powerOutsideOuterAngle = 0.f );

	void MakePointLight( const Vector3f& position, const Vector3f& direction, const Rgba& color, float minLightDistRadius, float maxLightDistRadius );
	void MakeSpotLight( const Vector3f& position, const Vector3f& direction, const Rgba& color, float minLightDistRadius, float maxLightDistRadius,
						float powerAtMinDist, float powerAtMaxDist,
						float innerAngleCosine, float outsideAngleCosine,
						float powerInsideInnerAngle, float powerOutsideOuterAngle );
	void MakeDirectionalLight( const Vector3f& position, const Vector3f& direction, const Rgba& color );

	static LightRegistryVector s_lightRegistry;

	bool m_isLightOn;
	Vector3f m_position; //"P"
	Vector3f m_direction; //"L_d" the direction from light, but use -L_d in code for direction to light.
	Rgba m_color;
	//float m_radius <=> m_maxLightDistance - m_minLightDistance.
	float m_minLightDistance;
	float m_maxLightDistance;
	float m_powerAtMinDistance;
	float m_powerAtMaxDistance;
	float m_innerAngleCosine;
	float m_outerAngleCosine;
	float m_powerInsideInnerAngle;
	float m_powerOutsideOuterAngle;
	int m_lerpTrickIsDirectional; //Only 1 for directional light, to use mix( (P - F_p).normalized, -L_d, m_lerpTrick ) instead of branching in shader code.
		//So for directional light, uses -L_d, but for anything else it uses the latter.
//	int m_lerpTrickIsSpotLight; //Because spot lights need to only use atten by angle. 
		//Removed as setting power inside/outside inner/outer angle := 1 for non-spotlights does same thing.

	static const float s_renderRadius;
};
