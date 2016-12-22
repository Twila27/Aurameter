#version 410 core

uniform sampler2D uTexDiffuse;

in vec2 passUV0;

//You can add more out vec4 colors here so long as your FBO has multiple color render targets.
out vec4 outColor; 
	//The order they are declared == the order they are bound to the FBO's render targets.

void main()
{
	vec2 resolution = textureSize( uTexDiffuse, 0 );
	float resolutionS = float( resolution.s );
	float resolutionT = float( resolution.t );
	vec4 diffuse = texture( uTexDiffuse, passUV0 );
	
	//Offsets / stride between texels.
	vec2 stp0 = vec2( 1.0/resolutionS, 0.0 );
	vec2 st0p = vec2( 0.0, 1.0/resolutionT );
	vec2 stpp = vec2( 1.0/resolutionS, 1.0/resolutionT );
	vec2 stpm = vec2( 1.0/resolutionS, -1.0/resolutionT );

	//3x3 pixel colors next. 9 samples.
	vec3 i00 = texture( uTexDiffuse, passUV0 ).rgb;
	vec3 im1m1 = texture( uTexDiffuse, passUV0-stpp ).rgb;
	vec3 ip1p1 = texture( uTexDiffuse, passUV0+stpp ).rgb;
	vec3 im1p1 = texture( uTexDiffuse, passUV0-stpm ).rgb;
	vec3 ip1m1 = texture( uTexDiffuse, passUV0+stpm ).rgb;
	vec3 im10 = texture( uTexDiffuse, passUV0-stp0 ).rgb;
	vec3 ip10 = texture( uTexDiffuse, passUV0+stp0 ).rgb;
	vec3 i0m1 = texture( uTexDiffuse, passUV0-st0p ).rgb;
	vec3 i0p1 = texture( uTexDiffuse, passUV0+st0p ).rgb;
	
	//ADD THE MATRIX FORM OF THE BELOW!
	vec3 color = vec3( 0.0 );
	color += 1.0 * ( im1m1 + ip1m1 + ip1p1 + im1p1 );
	color += 2.0 * ( im10 + ip10 + i0m1 + i0p1 );
	color += 4.0 * ( i00 );
	color /= 16;
	outColor = vec4( color, 1.0 );
}