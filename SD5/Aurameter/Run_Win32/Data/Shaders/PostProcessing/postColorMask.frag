#version 410 core

uniform sampler2D uTexDiffuse;
uniform vec4 uColorMask;

in vec2 passUV0;

//You can add more out vec4 colors here so long as your FBO has multiple color render targets.
out vec4 outColor; 
	//The order they are declared == the order they are bound to the FBO's render targets.

void main()
{
	vec4 diffuse = texture( uTexDiffuse, passUV0 );
	outColor = diffuse * uColorMask;
}