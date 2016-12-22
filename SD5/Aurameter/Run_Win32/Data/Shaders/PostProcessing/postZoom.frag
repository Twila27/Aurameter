#version 410 core

uniform sampler2D uTexDiffuse;
uniform float uUnwrappedTimer;
uniform float uWrappingTimer;
uniform float uWrappingTimerDuration;

in vec2 passUV0;

//You can add more out vec4 colors here so long as your FBO has multiple color render targets.
out vec4 outColor; 
	//The order they are declared == the order they are bound to the FBO's render targets.

const float PI = 3.14159265;
	
void main()
{	
	vec2 uv = passUV0;
	vec2 xy = uv;
	xy = 2.0 * xy - 1.0; // map to [-1,1] square
	float threshold = ( uWrappingTimer / uWrappingTimerDuration ); //uUnwrappedTimer
	xy += threshold * sin(PI*xy);
	uv = ( xy + 1.0 ) / 2.0; // map back to [0,1] square
	vec3 diffuse = texture( uTexDiffuse, uv ).rgb;
	outColor = vec4( diffuse, 1.0 );
}