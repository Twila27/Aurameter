//Fixed-vertex format or fixed-format pipeline.

#version 410 core

uniform int uShowTangent;
uniform int uShowBitangent;

uniform int uUseFog;
uniform int uUseDiffuse;
uniform int uUseSpecular;
uniform int uUseAmbientAndDOT3;
uniform int uUseAmbientOcclusion;
uniform int uUseEmissive;
uniform int uUseFalloffForSpecular;
uniform int uUseFalloffForAmbientAndDOT3;
uniform int uUseTime;
uniform float uWrappingTimer;

uniform sampler2D uTexDiffuse;
uniform sampler2D uTexNormal;
uniform sampler2D uTexSpecular;
uniform sampler2D uTexEmissive;
uniform sampler2D uTexAmbientOcclusion;
uniform sampler2D uTexHeights;

uniform vec4 uAmbientLight;
uniform vec3 uLightPosition; //Position of light in world.

uniform float uSpecularIntensity; //Extra control.
uniform float uLightIntensity; //For falloff.
uniform vec4 uLightColor;

uniform vec4 uFogColor;
uniform float uFogMinDist;
uniform float uFogMaxDist;

uniform float uSpecularPower;
uniform vec3 uCameraPosition;

//These are the attributes passed in from the vertex shader.
in vec3 passWorldSpacePosition;
in vec4 passthroughColor; //Must share names with .vert to get match.
in vec2 passUV0; //0 in case we want to add more pairs of UVs later.
in vec3 passTangent;
in vec3 passBitangent;

out vec4 finalColor; //The final output of frag shader.

vec3 CalculateNormalizedNormal( vec2 offsetUV )
{
	vec3 mapNormal = texture( uTexNormal, offsetUV ).xyz;
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
		//Now the mapNormal is valid for any uModel orientation.

	return mapNormal;
}


mat3 CalculateTangentSpaceTransformTBN()
{
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
	
	return tbn;
}

vec4 ColorFromVector( vec3 v ) //Map a normalized 0-1 vector to RGB colorspace.
{
	return vec4((v + vec3(1.0f)) * .5f, 1.0f); 
		//Convert a unit vector into color space.
		//This lets us validate our vector values.
		//In this system, a hue of 0 == -1, 127 == 0, 255 == +1.
}

vec3 CalculateLightForFragment( vec3 normal ) //Ambient + dot3 terms.
{
	vec3 ambientLightIntensity = uAmbientLight.rgb * uAmbientLight.a; 
		//Ambient term to "fake global illumination".
		//Alpha == intensity of ambient light.
	
	vec3 dirToLight = uLightPosition - passWorldSpacePosition;
	float inverseDistToLight = dot( dirToLight, dirToLight );
	inverseDistToLight = inversesqrt(inverseDistToLight);
	
	dirToLight = normalize(dirToLight);
	float falloffMultiplier = inverseDistToLight * inverseDistToLight;
	float falloffPower = uLightIntensity * falloffMultiplier; 
		//Falloff: scales by how far the light is from surface (via passWorldSpacePosition, lightPosition).
		//Should generally be > 1, e.g. 4.
		//Recall from Cline's undergrad course that an inverse square falloff tends to look best!
		
	//DOT3 operation:	
	float nDotL = clamp( dot( normal, dirToLight ), 0.0f, 1.0f );
		//Clamped to prevent it from subtracting if light goes behind face.
		//Be sure that dirToLight has been normalized.
	vec3 dot3LightIntensity = vec3(nDotL) * uLightColor.rgb;
	if ( uUseFalloffForAmbientAndDOT3 > 0 )
		dot3LightIntensity *= falloffPower;
		
	//Result: ambient term plus dot3 term.
	vec3 totalLightIntensity = ambientLightIntensity + dot3LightIntensity;
	vec3 saturatedLightIntensity = clamp(totalLightIntensity, vec3(0.0f), vec3(1.0f));
	return saturatedLightIntensity;
}

vec3 CalculateSpecularFactor( vec3 normal, vec2 offsetUV ) //How much light reflects into my eye.
{
	float specularIntensity = texture( uTexSpecular, offsetUV ).r;

	vec3 dirToLight = (uLightPosition - passWorldSpacePosition);
	
	float power = dot( dirToLight, dirToLight );
	dirToLight *= inversesqrt(power);
	
	vec3 dirToEye = normalize(uCameraPosition - passWorldSpacePosition);
	
	vec3 halfVector = dirToLight + dirToEye;
	halfVector = normalize(halfVector);
	
	float halfDotNormal = clamp( dot( halfVector, normal ), 0.0f, 1.0f ); 
		//There's a reflect func in GLSL, but Blinn-Phong is quicker, uses the half.
		//Clamped to prevent it from subtracting if light goes behind face.
	float intensity = pow( halfDotNormal, uSpecularPower ) * specularIntensity * uSpecularIntensity;//* uSpecularIntensity;
	
	power = clamp(uLightIntensity / power, 0.0f, 1.0f);
	
//	float inverseDistToLight = dot( dirToLight, dirToLight );
//	inverseDistToLight = inversesqrt(inverseDistToLight);
//	dirToLight *= inverseDistToLight; //same thing as normalizing.
//	float falloffPower = uLightIntensity * (inverseDistToLight * inverseDistToLight); //Should generally be > 1, e.g. 4.
//
	
	vec4 color = uLightColor * intensity * power;
//	if ( uUseFalloffForSpecular > 0 )
//		color *= falloffPower;
	return color.rgb;
}

float CalculateFogScale( void )
{
	//Assuming all distances are non-negative.
	vec3 dirToEye = uCameraPosition - passWorldSpacePosition;
	
	float distSquaredFromFragmentToCamera = dot( dirToEye, dirToEye );
	float fogMinDistSquared = uFogMinDist * uFogMinDist;
	float fogMaxDistSquared = uFogMaxDist * uFogMaxDist;
	
	return ( fogMaxDistSquared - distSquaredFromFragmentToCamera ) / ( fogMaxDistSquared - fogMinDistSquared );
}

void main( void )
{	
	//Parallax offset.
	float heightScale = 1.0f;
	mat3 tbn = CalculateTangentSpaceTransformTBN();
	vec3 tangentSpaceViewPos = uCameraPosition * tbn;
	vec3 tangentSpaceFragPos = passWorldSpacePosition * tbn;
	float height = texture(uTexHeights, passUV0).r * heightScale;
	height = 1.f - height; //Else the parts that are meant to stay solid move!
	vec3 viewDir = normalize(tangentSpaceViewPos - tangentSpaceFragPos);
	vec2 offsetUV = passUV0 + ( height * ( viewDir.xy ) ); // / viewDir.z ) );
		//Needed to have the uCamPos and fragment's passWorldSpacePosition in TBN space first.
	
	//Set initial values.
	vec3 normal = CalculateNormalizedNormal(offsetUV);
	vec4 diffuseTexColor = texture(uTexDiffuse, offsetUV);
	float ambientOcclusion = texture(uTexAmbientOcclusion, offsetUV).r;
	float emissiveStrength = texture(uTexEmissive, offsetUV).r;
	
//	finalColor = passthroughColor; 
	if ( uUseDiffuse > 0 )
		finalColor = diffuseTexColor;
	if ( uUseEmissive > 0 )
		finalColor += vec4( diffuseTexColor.rgb * emissiveStrength, 1.0f );
	if ( uUseAmbientAndDOT3 > 0 )
		finalColor *= vec4( CalculateLightForFragment(normal) * ambientOcclusion, 1.0f ); //DOT3 and ambient term use.
	if ( uUseSpecular > 0 )
		finalColor += vec4( CalculateSpecularFactor(normal, offsetUV), 1.0f ); //Metallic and shine effect.
	if ( uUseFog > 0 ) //Fog applied last.
		finalColor = mix( uFogColor, finalColor, CalculateFogScale() );
		
	//Because specular will send it way over 1, clamping b/t 0 and 1 called saturate() in DirectX.
	finalColor = clamp( finalColor, vec4(0.0f), vec4(1.0f) ); 
	
//	finalColor = ColorFromVector( CalculateLightForFragment(normal) );
	if ( uShowTangent > 0 )
		finalColor = ColorFromVector(passTangent);
	if ( uShowBitangent > 0 )
		finalColor = ColorFromVector(passBitangent);	
}