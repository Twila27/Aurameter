#version 410 core

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

in vec3 inPosition;
in vec2 inUV0;

out vec2 passUV0;

void main()
{
	passUV0 = inUV0;
	passUV0 *= -1.f; //OpenGL texture y-flip, else upside-down. Do same for tangent and bitangent.
	
	vec4 pos = vec4( inPosition, 1.f );
	pos = pos * uModel * uView * uProj;
	
	gl_Position = pos;
}
