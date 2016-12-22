//Fixed-vertex format or fixed-format pipeline.

#version 410 core

uniform float uWrappingTimer;

uniform sampler2D uTexDiffuse;
uniform sampler2D uTexNormal;
uniform sampler2D uTexSpecular;
uniform sampler2D uTexEmissive;
uniform sampler2D uTexAmbientOcclusion;

uniform vec4 uAmbientLight;
uniform vec3 uLightPosition; //Position of light in world.

uniform float uSpecularExponent;
uniform float uSpecularIntensity; //Extra control.
uniform float uLightIntensity; //For falloff.
uniform vec3 uCameraPosition;
uniform vec4 uLightColor;

//Be sure to match to clear screen color if seeking transparent look!
uniform vec4 uFogColor;
uniform float uFogMinDist;
uniform float uFogMaxDist;


//These are the attributes passed in from the vertex shader.
in vec2 passUV0; //0 in case we want to add more pairs of UVs later.
in vec3 passTangent;
in vec3 passBitangent;
in vec3 passWorldSpacePosition;
in vec4 passthroughColor; //Must share names with .vert to get match.

out vec4 finalColor; //The final output of frag shader.

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
		//Now the mapNormal is valid for any uModel orientation.

	return mapNormal;
}

vec4 ColorFromVector( vec3 v ) //Map a normalized 0-1 vector to RGB colorspace.
{
	return vec4((v + vec3(1.0f)) * .5f, 1.0f); 
		//Convert a unit vector into color space.
		//This lets us validate our vector values.
		//In this system, a hue of 0 == -1, 127 == 0, 255 == +1.
}

vec3 CalculateLightForFragment( vec3 normal ) //DOT3 term.
{	
	vec3 dirToLight = uLightPosition - passWorldSpacePosition;
	float inverseDistToLight = dot( dirToLight, dirToLight );
	inverseDistToLight = inversesqrt(inverseDistToLight);
	
	dirToLight = normalize(dirToLight);
	float falloffMultiplier = inverseDistToLight * inverseDistToLight;
	float falloffPower = uLightIntensity * falloffMultiplier; 
		//Falloff: scales by how far the light is from surface (via passWorldSpacePosition, lightPosition).
		//Recall from Cline's undergrad course that an inverse square falloff tends to look best!
		
	//DOT3 operation:	
	float nDotL = clamp( dot( normal, dirToLight ), 0.0f, 1.0f );
		//Clamped to prevent it from subtracting if light goes behind face.
		//Be sure that dirToLight has been normalized.
	vec3 dot3LightIntensity = vec3(nDotL) * uLightColor.rgb;
	dot3LightIntensity *= falloffPower;
		
	//Result: ambient term plus dot3 term.
	vec3 totalLightIntensity = dot3LightIntensity;
	vec3 saturatedLightIntensity = clamp(totalLightIntensity, vec3(0.0f), vec3(1.0f));
	return saturatedLightIntensity;
}

vec3 CalculateSpecularFactor( vec3 normal ) //How much light reflects into my eye.
{
	float specularIntensity = texture( uTexSpecular, passUV0 ).r;

	vec3 dirToLight = (uLightPosition - passWorldSpacePosition);
	
	float power = dot( dirToLight, dirToLight );
	dirToLight *= inversesqrt(power); //Same as normalize.
	
	vec3 dirToEye = normalize(uCameraPosition - passWorldSpacePosition);
	
	vec3 halfVector = dirToLight + dirToEye;
	halfVector = normalize(halfVector);
	
	float halfDotNormal = clamp( dot( halfVector, normal ), 0.0f, 1.0f ); 
		//There's a reflect func in GLSL, but Blinn-Phong is quicker, uses the half.
		//Clamped to prevent it from subtracting if light goes behind face.
	float intensity = pow( halfDotNormal, uSpecularExponent ) * specularIntensity * uSpecularIntensity;
	
	power = clamp(uLightIntensity / power, 0.0f, 1.0f);

	
	vec4 color = uLightColor * intensity * power;
	
	float inverseDistToLight = dot( dirToLight, dirToLight );
	inverseDistToLight = inversesqrt(inverseDistToLight);
	float falloffPower = uLightIntensity * (inverseDistToLight * inverseDistToLight);
	
	color *= falloffPower;
	
	return color.rgb;
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
	vec4 diffuseTexColor = texture(uTexDiffuse, passUV0);
	vec3 ambientColor = uAmbientLight.rgb * uAmbientLight.a;
	float ambientOcclusion = texture(uTexAmbientOcclusion, passUV0).r;
	float emissiveStrength = texture(uTexEmissive, passUV0).r;
	vec3 emissiveColor = diffuseTexColor.rgb * emissiveStrength;
	vec3 dot3 = CalculateLightForFragment(normal);
	
	//Lighting equation, Blinn-Phong because we used Blinn half-vector and Phong exponent specular shading:
	vec3 totalSurfaceLight = clamp( ( ambientOcclusion * ( ambientColor + dot3 ) ) + emissiveColor, vec3(0.0), vec3(1.0) );
	vec4 surfaceColor = diffuseTexColor * vec4( totalSurfaceLight, 1.0 ) + vec4( CalculateSpecularFactor(normal), 0.0 );
	finalColor = clamp( surfaceColor, 0.0, 1.0 ); //Because specular will send it way over vec4(1).	

	//Fog always comes last.
	finalColor = mix( uFogColor, finalColor, CalculateFogScale() );
}