//Fixed-vertex format or fixed-format pipeline.

#version 410 core

#define MAX_LIGHTS 16

uniform sampler2D uTexDiffuseSkin;

uniform int		uLightIsDirectionalFlags[MAX_LIGHTS];
uniform int		uLightIsSpotLightFlags[MAX_LIGHTS];
uniform vec3 	uLightPositions[MAX_LIGHTS];
uniform vec3 	uLightDirections[MAX_LIGHTS];
uniform vec4 	uLightColors[MAX_LIGHTS];
uniform float 	uLightMinDistances[MAX_LIGHTS];
uniform float 	uLightMaxDistances[MAX_LIGHTS];
uniform float 	uLightPowersAtMinDists[MAX_LIGHTS];
uniform float 	uLightPowersAtMaxDists[MAX_LIGHTS];
uniform float 	uLightInnerAngleCosines[MAX_LIGHTS];
uniform float 	uLightOuterAngleCosines[MAX_LIGHTS];
uniform float 	uLightPowersInsideInnerAngles[MAX_LIGHTS];
uniform float 	uLightPowersOutsideOuterAngles[MAX_LIGHTS];

uniform float uWrappingTimer;

uniform sampler2D uTexDiffuse;
uniform sampler2D uTexNormal;
uniform sampler2D uTexSpecular;
uniform sampler2D uTexEmissive;
uniform sampler2D uTexAmbientOcclusion;

uniform vec4 uAmbientLight;

uniform vec3 uCameraPosition;
uniform float uSpecularExponent;
uniform float uSpecularIntensity; //Extra control.

//Be sure to match to clear screen color if seeking transparent look!
uniform vec4 uFogColor;
uniform float uFogMinDist;
uniform float uFogMaxDist;


//These are the attributes passed in from the vertex shader.
in vec2 passUV0;
in vec2 passUV1;
in vec2 passUV2;
in vec2 passUV3;
in vec3 passWorldSpacePosition;
in vec3 passTangent;
in vec3 passBitangent;
in vec3 passNormal;
in vec4 passthroughColor; //Must share names with .vert to get match.

out vec4 finalColor; //The final output of frag shader.


vec4 ColorFromVector( vec3 v ) //Map a normalized 0-1 vector to RGB colorspace.
{
	return vec4((v + vec3(1.0f)) * .5f, 1.0f); 
		//Convert a unit vector into color space.
		//This lets us validate our vector values.
		//In this system, a hue of 0 == -1, 127 == 0, 255 == +1.
}


float CalculateDistanceToLight( int lightIndex ) //To light from this fragment, AKA "d" in class notes.
{
	vec3 unnormalizedDirToLight = uLightPositions[lightIndex] - passWorldSpacePosition;
	float dist = length( unnormalizedDirToLight );
	return mix( dist, dot( unnormalizedDirToLight, -uLightDirections[lightIndex] ), uLightIsDirectionalFlags[lightIndex] );
		//i.e. Returns planar distance for directional lights.
}


vec3 CalculateNormalizedDirectionToLight( int lightIndex ) //"L" in class notes.
{
	vec3 dirToLight = ( uLightPositions[lightIndex] - passWorldSpacePosition );
	float distanceToLight = CalculateDistanceToLight( lightIndex );
	
	vec3 unnormalizedDir = mix( dirToLight / distanceToLight, -uLightDirections[lightIndex], uLightIsDirectionalFlags[lightIndex] );
		//i.e. Returns -uLightDirection[i] for directional lights only.
	
	return unnormalizedDir * inversesqrt( dot( unnormalizedDir, unnormalizedDir ) ); 
		//Product above does normalization: " v / sqrt( |v|^2 ) " == v / |v|.
}


float CalculateAttenuationForLight( vec3 normal, int lightIndex )
{
	//Angle between light's full-on direction ray and the ray to-surface-FROM-light.
	float angle = dot( uLightDirections[lightIndex], -CalculateNormalizedDirectionToLight(lightIndex) );
		//e.g. the light shines straight down, but I'm 45 degrees off to its side.

	float attenByAngle = smoothstep( uLightInnerAngleCosines[lightIndex], uLightOuterAngleCosines[lightIndex], angle );
	attenByAngle = mix( uLightPowersInsideInnerAngles[lightIndex], uLightPowersOutsideOuterAngles[lightIndex], attenByAngle );

	//Could literally shove the below lines into the mix() to only ever execute half, but... super messy then?
	float attenByDistance = smoothstep( uLightMinDistances[lightIndex], uLightMaxDistances[lightIndex], CalculateDistanceToLight(lightIndex) );
	attenByDistance = mix( uLightPowersAtMinDists[lightIndex], uLightPowersAtMaxDists[lightIndex], attenByDistance );
	
	return mix( attenByAngle * attenByDistance, 1.0, float(uLightIsDirectionalFlags[lightIndex]) );
}


vec3 CalculateNormalizedNormal( void )
{
	vec3 mapNormal = texture( uTexNormal, passUV0 ).xyz;
		//.x == left to right along u-dir, .y == vertically along v-dir.
		//z points out from texture
		//i.e. -1 to 1, -1 to 1, 0 to 1.
		//(since you never have an inward-z, a 0-1 range for it increases our z-precision).
	
	vec3 surfaceTangent = normalize(passTangent);
	vec3 surfaceBitangent = normalize(passBitangent);
	vec3 surfaceNormal = cross( surfaceTangent, surfaceBitangent ); 
		//For LHS, bitangent-to-tangent, for RHS, tangent-to-bitangent.
	surfaceBitangent = cross( surfaceNormal, surfaceTangent ); 
		//Tangent first if LHS, normal first if RHS.
		//I don't invert z-axis in the perspective projection matrix, so RHS.
	
	//Construct 3x3 TBN matrix, mat3(i,j,k) sets the COLUMNS to ijk.
	mat3 tbn = mat3( surfaceTangent, surfaceBitangent, surfaceNormal );
		//If you multiply mapNormal on the right of TBN mat3, need to transpose.
		//This is because mat3's ctor inserts its arguments as columns.
	tbn = transpose( tbn ); 
		//But this transpose is separate, meant to reverse the transform direction from/to texture space.
	
	mapNormal = ( mapNormal * vec3(2.0f, 2.0f, 1.0f)) - vec3(1.0f, 1.0f, 0.0f);
		//Maps between 0-1 and -1 to 1 for u, v. Preserves the 0-1 z-direction of texture space.
	mapNormal = normalize(mapNormal); 
		//Now it'll work regardless of orientation of the model, unlike before.
	mapNormal *= tbn; //Actual transform.
		//Moves normal from texture space to world space.
		//Now the mapNormal is valid for any uModel orientation.

	return mapNormal;
}


vec3 CalculateDOT3ForLight( vec3 normal, int lightIndex )
{	
	vec3 dirToLight = CalculateNormalizedDirectionToLight( lightIndex );
			
	//DOT3 operation: nDotL <=> angle of incidence!
	float nDotL = max( dot( normal, dirToLight ), 0.0f ) * CalculateAttenuationForLight(normal, lightIndex);
		//max() keeps it from becoming subtractive for negative angles.
		//Be sure that dirToLight has been normalized.

	vec3 dot3LightIntensity = uLightColors[lightIndex].rgb * vec3(nDotL);
	return dot3LightIntensity;
		
	//Wasn't actually clamped in the example, maybe since ambient's not added to it?
	vec3 saturatedLightIntensity = clamp(dot3LightIntensity, vec3(0.0f), vec3(1.0f));
	return saturatedLightIntensity;
}


vec3 CalculateSpecularForLight( vec3 normal, int lightIndex ) //How much light reflects into my eye.
{
	float specularIntensity = texture( uTexSpecular, passUV0 ).r;

	vec3 dirToEye = normalize(uCameraPosition - passWorldSpacePosition);
	vec3 dirToLight = CalculateNormalizedDirectionToLight( lightIndex ); 
	float inverseDistToLight = dot( dirToLight, dirToLight );
	inverseDistToLight = inversesqrt(inverseDistToLight);
	float falloffPower = CalculateAttenuationForLight(normal, lightIndex) * (inverseDistToLight * inverseDistToLight);
	
	vec3 halfVector = normalize(dirToLight + dirToEye);
	
	//nDotHalf <=> still angle of incidence!
	float halfDotNormal = max( dot( normal, halfVector ), 0.0f ); 
		//max() keeps it from becoming subtractive for negative angles.
		//There's a reflect func in GLSL, but Blinn-Phong is quicker, uses the half.
	float scaledSpecularIntensity = specularIntensity * uSpecularIntensity;
	float attenuatedSpecularIntensity = pow( halfDotNormal, uSpecularExponent ) * scaledSpecularIntensity * falloffPower;
		//Not 1:1, but specular intensity is where the Phong exponent comes in.
			
	//Mix terms with light's base color.
	return uLightColors[lightIndex].rgb * attenuatedSpecularIntensity;
}


float CalculateFogScale( void )
{
	//Assuming all distances are non-negative.
	vec3 dirToEye = uCameraPosition - passWorldSpacePosition;

	float distSquaredFromFragmentToCamera = dot( dirToEye, dirToEye );
	float fogMinDistSquared = uFogMinDist * uFogMinDist;
	float fogMaxDistSquared = uFogMaxDist * uFogMaxDist;
	
	float fogRadius = fogMaxDistSquared - fogMinDistSquared;
	float distToCam = fogMaxDistSquared - distSquaredFromFragmentToCamera;
	
	return clamp( ( distToCam / fogRadius ), 0.0, 1.0 );
}


void main( void )
{	
	//Set initial values.
	vec3 normal = CalculateNormalizedNormal();
	vec4 diffuseTexColor = texture(uTexDiffuseSkin, passUV0);
	vec3 ambientColor = uAmbientLight.rgb * uAmbientLight.a;
	float ambientOcclusion = texture(uTexAmbientOcclusion, passUV0).r;
	float emissiveStrength = texture(uTexEmissive, passUV0).r;
	vec3 emissiveColor = diffuseTexColor.rgb * emissiveStrength;
	
	//Loop over all lights, adding the dot3 and spec of each to totalSurfaceLight and totalSpecularLight respectively.
	vec3 totalSurfaceLight = vec3(0.0);
	vec3 totalSpecularLight = vec3(0.0);
	for ( int i = 0; i < MAX_LIGHTS; i++ )
	{
		totalSurfaceLight += CalculateDOT3ForLight(normal, i);
		totalSpecularLight += CalculateSpecularForLight(normal, i);
	}
	
	//Lighting equation, Blinn-Phong because we used Blinn half-vector and Phong exponent specular shading:
	totalSurfaceLight = clamp( ( ambientOcclusion * ( ambientColor + totalSurfaceLight ) ) + emissiveColor, vec3(0.0), vec3(1.0) );
	vec4 surfaceColor = diffuseTexColor * vec4( totalSurfaceLight, 1.0 ) + vec4( totalSpecularLight, 0.0 );
	finalColor = clamp( surfaceColor, vec4(0.0), vec4(1.0) ); //Because specular will send it way over vec4(1).	
	
	//Fog always comes last.
	finalColor = mix( uFogColor, finalColor, CalculateFogScale() );
	
	finalColor = diffuseTexColor;
}