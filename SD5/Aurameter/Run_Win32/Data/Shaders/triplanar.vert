#version 410 core

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

in vec3 inPosition;
in vec4 inColor;
in vec2 inUV0; //0 in case we want to add more pairs of UVs later.

in vec3 inTangent;
in vec3 inBitangent;

out vec4 passthroughColor;
out vec2 passUV0;
out vec3 passPosition;
out vec3 passWorldSpacePosition;

out vec3 passTangent;
out vec3 passBitangent;


void main( void )
{
	passthroughColor = inColor;
	
	passUV0 = inUV0;
	
	passTangent = (vec4(inTangent, 0.0f) * uModel).xyz; //Right-multiply like we do for pos below.
	passBitangent = (vec4(inBitangent, 0.0f) * uModel).xyz; //Right-multiply like we do for pos below.
	
	vec4 pos = vec4( inPosition, 1.0f );
		//Setting xyz to our input, and w-coordinate to 1.
		//MUST set Clip space: [-1,1]x[-1,1]x[-1,1]x[w]. After transformations by matrices but pre-w.
		//gl_Position is a built-in out-type.
		
	passWorldSpacePosition = (pos * uModel).xyz;
		//Make fit your v*MVP order.
		
	pos = pos * uModel * uView * uProj; 
		//Ephanov's style: vert's rows * mats' cols ("row-major").
		
	//pos = uProj * uView * uModel * pos; 
		//mats' rows * vert's cols ("col-major").
		//Recall GL_TRUE in glUniformMatrix4fv transposes the 4x4's, check that if results are off.
		
	gl_Position = pos;
}