#version 410 core

uniform sampler2D uTexDiffuse;
uniform float uUnwrappedTimer;
uniform float uWrappingTimer;
uniform float uWrappingTimerDuration;

in vec2 passUV0;

//You can add more out vec4 colors here so long as your FBO has multiple color render targets.
out vec4 outColor; 
	//The order they are declared == the order they are bound to the FBO's render targets.
	
//Unlike brightness and contrast, saturation and sharpness do best when t isn't normalized--we generally want t > 1.
void main()
{	
	vec3 diffuse = texture( uTexDiffuse, passUV0 ).rgb;
	const vec3 luminanceWeightVector = vec3( .2125, .7174, .0721 );
	float luminance = dot( diffuse, luminanceWeightVector );
	
	vec3 saturatedDiffuse = mix( diffuse, vec3(luminance), uWrappingTimer );
	outColor = vec4( saturatedDiffuse, 1.0 );
}