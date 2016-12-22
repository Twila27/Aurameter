#version 410 core

uniform sampler2D uTexDiffuse;

in vec2 passUV0;

//You can add more out vec4 colors here so long as your FBO has multiple color render targets.
out vec4 outColor; 
	//The order they are declared == the order they are bound to the FBO's render targets.

void main()
{
	vec4 diffuse = texture( uTexDiffuse, passUV0 );
	
	//Take it to gray.
	vec4 toGray = vec4(.2126, .7152, .0722, 0); //Eye more sensitivity to green.
	float gray = dot( diffuse, toGray );
		//This is actually not just "gray" but the fragment "luminance". Float values == sRGB luminance weight vector.
	
	//Colors to tone map the bands of gray to.
	vec3 r = vec3( .9, .2, .19 );
	vec3 p = vec3( 0, .2, .33 );
	vec3 l = vec3( .95, .8, .7 );
	vec3 b = vec3( .4, .7, .8 );

	//Actual tone mapping.
	vec3 color;
	if ( gray < .1 )
		color = p;
	else if ( gray < .4 )
		color = r;
	else if ( gray < .7 )
		color = b;
	else
		color = l;
		
	outColor = vec4( color, 1.0 );
}