#version 410 core

uniform sampler2D uTexDiffuse;
uniform float uHueShift;
uniform float uWrappingTimer;

in vec2 passUV0;

//You can add more out vec4 colors here so long as your FBO has multiple color render targets.
out vec4 outColor; 
	//The order they are declared == the order they are bound to the FBO's render targets.

vec3 convertRGB2HSV( vec3 colorRGB ) //HSV: hue, saturation, value colorspace.
{
	float h, s, v;
	
	float r = colorRGB.r;
	float g = colorRGB.g;
	float b = colorRGB.b;
	
	float maxV = max( r, max( g, b ) );
	v = maxV;
	float minV = min( r, min( g, b ) );
	
	float deltaV = maxV - minV;
	
	s = mix( deltaV/maxV, 0.0, maxV == 0.0f );
		//originally: if ( maxV == 0.0 ) s = 0.0; else s = deltaV / maxV;
	
	if ( s == 0.0 ) h = 0.0; // "actually, h is indeterminate for zero saturation"
	else 
	{
		if ( r == maxV ) h = ( g - b ) / deltaV;
		else if ( g == maxV ) h = 2.0 + ( b - r ) / deltaV;
		else if ( b == maxV ) h = 4.0 + ( r - g ) / deltaV;
		
		h *= 60.0;
		
		if ( h < 0.0 ) h += 360.0; // keep h in 0-359 range.	
	}
	return vec3( h, s, v );
}
	

vec3 convertHSV2RGB( vec3 colorHSV )
{
	float h = colorHSV.x;
	float s = colorHSV.y;
	float v = colorHSV.z;
	
	if ( s == 0.0 ) return vec3( v, v, v ); // "achromatic: zero saturation", so return grayed out color.
	else // "chromatic case"
	{
		if ( h > 360.0 ) h = 360.0; // "h must be within [0, 360)"
		if ( h < 0.0 ) h = 0.0;
		
		h /= 60.0;
		
		int k = int(h);
		float f = h - float(k);
		float p = v * ( 1.0 - s );
		float q = v * ( 1.0 - ( s * f ) );
		float t = v * ( 1.0 - ( s * ( 1.0 - f ) ) );
		
		if ( k == 0 ) return vec3( v, t, p );
		if ( k == 1 ) return vec3( q, v, p );
		if ( k == 2 ) return vec3( p, v, t );
		if ( k == 3 ) return vec3( p, q, v );
		if ( k == 4 ) return vec3( t, p, v );
		if ( k == 5 ) return vec3( v, p, q );
	}
}

	
//Hue Shift: convert RGB to HSV where hue (the H) is an angular function s.t. shifting <=> shifted HSV := (HSV + shift) mod 360, then convert back.
void main()
{
	vec4 diffuse = texture( uTexDiffuse, passUV0 );
	
	vec3 colorHSV = convertRGB2HSV( diffuse.rgb );
	
	colorHSV.x += uHueShift;
	colorHSV.x += uWrappingTimer * 12.f;
	
	//Simulate the mod 360 operation, note to come back and optimize this with mix(,,sign(colorHSV.x-360.0)), etc. Kept mod() since timer can > 720.
	colorHSV.x = mod( colorHSV.x, 360.0 );
		//When outside its range, black starts to weirdly creep into everything.
	
	//if ( colorHSV.x > 360.0 ) colorHSV.x -= 360.0;
	
	//if ( colorHSV.x < 0.0 ) colorHSV.x += 360.0;
	
	outColor.rgb = convertHSV2RGB( colorHSV );
	outColor.a = 1.0;
}