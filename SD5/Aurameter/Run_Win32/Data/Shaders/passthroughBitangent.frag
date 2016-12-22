#version 410 core

uniform vec4 uColor;

//These are the attributes passed in from the vertex shader.
in vec2 passUV0;
in vec3 inPosition;
in vec4 passthroughColor; //Must share names with .vert to get match.
in vec3 passBitangent;

out vec4 finalColor; //The final output of frag shader.

vec4 ColorFromVector( vec3 v ) //Map a normalized 0-1 vector to RGB colorspace.
{
	return vec4((v + vec3(1.0f)) * .5f, 1.0f); 
		//Convert a unit vector into color space.
		//This lets us validate our vector values.
		//In this system, a hue of 0 == -1, 127 == 0, 255 == +1.
}


void main( void )
{	
//	finalColor = passthroughColor * vec4( sin( gl_FragCoord.x ), cos( gl_FragCoord.y ) * .4f, 0.f, 1.f ); //Says to output a green hue for this fragment.
	
	finalColor = ColorFromVector( passBitangent ); //vec4(1.0f);
}