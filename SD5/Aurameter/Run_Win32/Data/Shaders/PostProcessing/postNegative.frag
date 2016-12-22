#version 410 core

uniform sampler2D uTexDiffuse;
uniform float uUnwrappedTimer;
uniform float uWrappingTimer;
uniform float uWrappingTimerDuration;

in vec2 passUV0;

//You can add more out vec4 colors here so long as your FBO has multiple color render targets.
out vec4 outColor; 
	//The order they are declared == the order they are bound to the FBO's render targets.
	
void main()
{	
	vec3 diffuse = texture( uTexDiffuse, passUV0 ).rgb;
	vec3 negativeDiffuse = vec3(1.0) - diffuse;
	outColor = vec4( negativeDiffuse, 1.0 );
}