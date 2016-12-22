#version 410 core

uniform vec4 uColor;

//These are the attributes passed in from the vertex shader.
in vec2 passUV0;
in vec3 inPosition;
in vec4 passthroughColor; //Must share names with .vert to get match.
in vec3 passWorldSpacePosition;

out vec4 finalColor; //The final output of frag shader.

vec4 ColorFromVector( vec3 v ) //Map a normalized 0-1 vector to RGB colorspace.
{
	return vec4((v + vec3(1.0f)) * .5f, 1.0f); 
		//Convert a unit vector into color space.
		//This lets us validate our vector values.
		//In this system, a hue of 0 == -1, 127 == 0, 255 == +1.
}

int Get1dNoiseUint( int positionX, int seed )
{
	const int BIT_NOISE1 = 0xB5297A4D; // 0b1011'0101'0010'1001'0111'1010'0100'1101;
	const int BIT_NOISE2 = 0x68E31DA4; // 0b0110'1000'1110'0011'0001'1101'1010'0100;
	const int BIT_NOISE3 = 0x1B56C4E9; // 0b0001'1011'0101'0110'1100'0100'1110'1001;

	int mangledBits = int(positionX);
	mangledBits *= BIT_NOISE1;
	mangledBits += seed;
	mangledBits ^= (mangledBits >> 8);
	mangledBits += BIT_NOISE2;
	mangledBits ^= (mangledBits << 8);
	mangledBits *= BIT_NOISE3;
	mangledBits ^= (mangledBits >> 8);
	return mangledBits;
}


int Get3dNoiseUint( int indexX, int indexY, int indexZ, int seed )
{
	const int PRIME1 = 198491317; // Large prime number with non-boring bits
	const int PRIME2 = 6542989; // Large prime number with distinct and non-boring bits
	return Get1dNoiseUint( indexX + (PRIME1 * indexY) + (PRIME2 * indexZ), seed );
}


float FastFloor( float f )
{
	if( f >= 0.f )
		return float(int(f));
	else
	{
		float f2 = float(int(f));
		if( f == f2 )
			return f2;
		else
			return f2 - 1.f;
	}
}


float SmoothStep5( float inputZeroToOne )
{
	float t = inputZeroToOne;
	return t * t * t * ( t * (t * 6.f - 15.f) + 10.f );
}	
float SmoothStep( float inputZeroToOne )
{
	return inputZeroToOne * inputZeroToOne * (3.f - (2.f * inputZeroToOne));
}

float Compute3dPerlinNoise( vec3 position, float scale, int numOctaves, float octavePersistence, float octaveScale, bool renormalize, int seed )
{
	int posX = int(position.x);
	int posY = int(position.y);
	int posZ = int(position.z);

	const float OCTAVE_OFFSET = 0.636764989593174f; // Translation/bias to add to each octave
	const float fSQRT_3_OVER_3 = 0.57735026918962576450914878050196f;
	
	const vec3 gradients[ 8 ] = vec3[ 8 ] // Traditional "12 edges" requires modulus and isn't any better.
	(
		vec3( +fSQRT_3_OVER_3, +fSQRT_3_OVER_3, +fSQRT_3_OVER_3 ), // Normalized unit 3D vectors
		vec3( -fSQRT_3_OVER_3, +fSQRT_3_OVER_3, +fSQRT_3_OVER_3 ), //  pointing toward cube
		vec3( +fSQRT_3_OVER_3, -fSQRT_3_OVER_3, +fSQRT_3_OVER_3 ), //  corners, so components
		vec3( -fSQRT_3_OVER_3, -fSQRT_3_OVER_3, +fSQRT_3_OVER_3 ), //  are all sqrt(3)/3, i.e.
		vec3( +fSQRT_3_OVER_3, +fSQRT_3_OVER_3, -fSQRT_3_OVER_3 ), // 0.5773502691896257645091f.
		vec3( -fSQRT_3_OVER_3, +fSQRT_3_OVER_3, -fSQRT_3_OVER_3 ), // These are slightly better
		vec3( +fSQRT_3_OVER_3, -fSQRT_3_OVER_3, -fSQRT_3_OVER_3 ), // than axes (1,0,0) and much
		vec3( -fSQRT_3_OVER_3, -fSQRT_3_OVER_3, -fSQRT_3_OVER_3 )  // faster than edges (1,1,0).
	);

	float totalNoise = float(0.f);
	float totalAmplitude = float(0.f);
	float currentAmplitude = float(1.f);
	float invScale = (1.f / scale);
	vec3 currentPos = vec3( posX * invScale, posY * invScale, posZ * invScale );

	for( int octaveNum = 0; octaveNum < numOctaves; ++ octaveNum )
	{
		// Determine random unit "gradient vectors" for surrounding corners
		vec3 cellMins = vec3( FastFloor( currentPos.x ), FastFloor( currentPos.y ), FastFloor( currentPos.z ) );
		vec3 cellMaxs = vec3( cellMins.x + 1.f, cellMins.y + 1.f, cellMins.z + 1.f );
		int indexWestX  = int(cellMins.x);
		int indexSouthY = int(cellMins.y);
		int indexBelowZ = int(cellMins.z);
		int indexEastX  = indexWestX  + 1;
		int indexNorthY = indexSouthY + 1;
		int indexAboveZ = indexBelowZ + 1;

		int noiseBelowSW = Get3dNoiseUint( indexWestX, indexSouthY, indexBelowZ, seed );
		int noiseBelowSE = Get3dNoiseUint( indexEastX, indexSouthY, indexBelowZ, seed );
		int noiseBelowNW = Get3dNoiseUint( indexWestX, indexNorthY, indexBelowZ, seed );
		int noiseBelowNE = Get3dNoiseUint( indexEastX, indexNorthY, indexBelowZ, seed );
		int noiseAboveSW = Get3dNoiseUint( indexWestX, indexSouthY, indexAboveZ, seed );
		int noiseAboveSE = Get3dNoiseUint( indexEastX, indexSouthY, indexAboveZ, seed );
		int noiseAboveNW = Get3dNoiseUint( indexWestX, indexNorthY, indexAboveZ, seed );
		int noiseAboveNE = Get3dNoiseUint( indexEastX, indexNorthY, indexAboveZ, seed );

		vec3 gradientBelowSW = gradients[ noiseBelowSW & 0x00000007 ];
		vec3 gradientBelowSE = gradients[ noiseBelowSE & 0x00000007 ];
		vec3 gradientBelowNW = gradients[ noiseBelowNW & 0x00000007 ];
		vec3 gradientBelowNE = gradients[ noiseBelowNE & 0x00000007 ];
		vec3 gradientAboveSW = gradients[ noiseAboveSW & 0x00000007 ];
		vec3 gradientAboveSE = gradients[ noiseAboveSE & 0x00000007 ];
		vec3 gradientAboveNW = gradients[ noiseAboveNW & 0x00000007 ];
		vec3 gradientAboveNE = gradients[ noiseAboveNE & 0x00000007 ];

		// Dot each corner's gradient with displacement from corner to position
		vec3 displacementFromBelowSW = vec3( currentPos.x - cellMins.x, currentPos.y - cellMins.y, currentPos.z - cellMins.z );
		vec3 displacementFromBelowSE = vec3( currentPos.x - cellMaxs.x, currentPos.y - cellMins.y, currentPos.z - cellMins.z );
		vec3 displacementFromBelowNW = vec3( currentPos.x - cellMins.x, currentPos.y - cellMaxs.y, currentPos.z - cellMins.z );
		vec3 displacementFromBelowNE = vec3( currentPos.x - cellMaxs.x, currentPos.y - cellMaxs.y, currentPos.z - cellMins.z );
		vec3 displacementFromAboveSW = vec3( currentPos.x - cellMins.x, currentPos.y - cellMins.y, currentPos.z - cellMaxs.z );
		vec3 displacementFromAboveSE = vec3( currentPos.x - cellMaxs.x, currentPos.y - cellMins.y, currentPos.z - cellMaxs.z );
		vec3 displacementFromAboveNW = vec3( currentPos.x - cellMins.x, currentPos.y - cellMaxs.y, currentPos.z - cellMaxs.z );
		vec3 displacementFromAboveNE = vec3( currentPos.x - cellMaxs.x, currentPos.y - cellMaxs.y, currentPos.z - cellMaxs.z );

		float dotBelowSW = dot( gradientBelowSW, displacementFromBelowSW );
		float dotBelowSE = dot( gradientBelowSE, displacementFromBelowSE );
		float dotBelowNW = dot( gradientBelowNW, displacementFromBelowNW );
		float dotBelowNE = dot( gradientBelowNE, displacementFromBelowNE );
		float dotAboveSW = dot( gradientAboveSW, displacementFromAboveSW );
		float dotAboveSE = dot( gradientAboveSE, displacementFromAboveSE );
		float dotAboveNW = dot( gradientAboveNW, displacementFromAboveNW );
		float dotAboveNE = dot( gradientAboveNE, displacementFromAboveNE );

		// Do a smoothed (nonlinear) weighted average of dot results
		float weightEast  = SmoothStep5( displacementFromBelowSW.x );
		float weightNorth = SmoothStep5( displacementFromBelowSW.y );
		float weightAbove = SmoothStep5( displacementFromBelowSW.z );
		float weightWest  = 1.f - weightEast;
		float weightSouth = 1.f - weightNorth;
		float weightBelow = 1.f - weightAbove;

		// 8-way blend (8 -> 4 -> 2 -> 1)
		float blendBelowSouth = (weightEast * dotBelowSE) + (weightWest * dotBelowSW);
		float blendBelowNorth = (weightEast * dotBelowNE) + (weightWest * dotBelowNW);
		float blendAboveSouth = (weightEast * dotAboveSE) + (weightWest * dotAboveSW);
		float blendAboveNorth = (weightEast * dotAboveNE) + (weightWest * dotAboveNW);
		float blendBelow = (weightSouth * blendBelowSouth) + (weightNorth * blendBelowNorth);
		float blendAbove = (weightSouth * blendAboveSouth) + (weightNorth * blendAboveNorth);
		float blendTotal = (weightBelow * blendBelow) + (weightAbove * blendAbove);
		float noiseThisOctave = 1.66666666f * blendTotal; // 3D Perlin is ~[-.6,.6]; map to ~[-1,1]

		// Accumulate results and prepare for next octave (if any)
		totalNoise += noiseThisOctave * currentAmplitude;
		totalAmplitude += currentAmplitude;
		currentAmplitude *= octavePersistence;
		currentPos *= octaveScale;
		currentPos.x += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		currentPos.y += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		currentPos.z += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		++ seed; // Eliminates octaves "echoing" each other (since each octave is uniquely seeded)
	}

	// Re-normalize total noise to within [-1,1] and fix octaves pulling us far away from limits
	if( renormalize && totalAmplitude > 0.f )
	{
		totalNoise /= totalAmplitude;				// Amplitude exceeds 1.0 if octaves are used
		totalNoise = (totalNoise * 0.5f) + 0.5f;	// Map to [0,1]
		totalNoise = SmoothStep( totalNoise );		// Push towards extents (octaves pull us away)
		totalNoise = (totalNoise * 2.0f) - 1.f;		// Map back to [-1,1]
	}

	return totalNoise;
}

vec4 CalcCloudValue( void )
{
	vec4 uSkyColor = vec4( 0, 0, 1, 1 );
	vec4 uCloudColor = vec4( 1 );
	float uBias = 0.f;
	float uNoiseScale = 1.f;
	
	const float PI = 3.14159265;
	
//	vec4 noiseValue = vec4( vec3( uNoiseScale * Compute3dPerlinNoise( inPosition, .5f, 2, .5f, 2.f, true, 0 ) ), 1.0 );
//	float sum = ( 3.* noiseValue.r + noiseValue.g + noiseValue.b + noiseValue.a - 2. ) / 2.;
	float sum = 0.5 * ( 1.0 + cos(PI * Compute3dPerlinNoise( inPosition, 1.f, 1, 1.f, 2.f, true, 0 )) );
	float t = clamp( uBias + sum, 0.0, 1.0 );
	
	vec3 color = mix( uSkyColor.rgb, uCloudColor.rgb, t );
	
	return vec4( color, 1.0 );
}	


void main( void )
{	
	vec4 uSkyColor = vec4( 0, 0, 1, 1 );
	vec4 uCloudColor = vec4( 1 );
	
//	finalColor = passthroughColor * vec4( sin( gl_FragCoord.x ), cos( gl_FragCoord.y ) * .4f, 0.f, 1.f ); //Says to output a green hue for this fragment.
	
//	finalColor = CalcCloudValue(); //vec4(1.0f);
	finalColor = mix( uSkyColor, uCloudColor, Compute3dPerlinNoise( passWorldSpacePosition, 1.f, 1, 1.f, 1.f, true, 0 ) * 0.5 + 1.0 );
	finalColor = ColorFromVector( vec3( Compute3dPerlinNoise( passWorldSpacePosition, .5f, 1, .5f, .5f, true, 0 ) * 0.5 + 1.0 ) );
	
}