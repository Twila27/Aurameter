#version 410 core

uniform sampler2D uTexDiffuse;

in vec2 passUV0;

//You can add more out vec4 colors here so long as your FBO has multiple color render targets.
out vec4 outColor; 
	//The order they are declared == the order they are bound to the FBO's render targets.

	
//Grayscale: replace each pixel component by the original pixel's luminance (i.e. dot value with below sRGB luminance's weight vector).
void main()
{
	vec4 diffuse = texture( uTexDiffuse, passUV0 );
	vec3 luminanceWeightVector = vec3( .2126, .7152, .0722 );
	float luminance = dot( diffuse.rgb, luminanceWeightVector );
	outColor = vec4( vec3( luminance ), 1. );
}