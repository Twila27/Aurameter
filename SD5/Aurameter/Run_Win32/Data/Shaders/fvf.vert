//Fixed-vertex format or fixed-format pipeline.

#version 410 core //Specify the OpenGL version first. Saved as passthrough.vert as it passes on data. Use the NShader.vsix--.vert and .vs will bring it up.

uniform mat4 uModel; //Update every frame, the object you're drawing.
uniform mat4 uView;
uniform mat4 uProj; //Can sometimes be composite uViewProj.

//Vertex shaders must take in all vertex shaders.
in vec3 inPosition; //vec3 is a GLSL built-in, and "in" defines it as an input.
in vec4 inColor;
in vec2 inUV0; //0 in case we want to add more pairs of UVs later.

out vec4 passthroughColor;
out vec2 passUV0;

void main( void )
{
	//gl_Position = vec4( inPosition, 1.0f ) ; //Says that this vertex has this position post-vertex-shader.
		//Setting xyz to our input, and w-coordinate to 1.
		//MUST set Clip space: [-1,1]x[-1,1]x[-1,1]x[w]. After transformations by matrices but pre-w.
		//gl_Position is a built-in out-type.
		
	passthroughColor = inColor;
	
	passUV0 = inUV0;
	
	vec4 pos = vec4( inPosition, 1.0f );
	pos = pos * uModel * uView * uProj; //Ephanov's style: vert's rows * mats' cols ("row-major").
	//pos = uProj * uView * uModel * pos; //mats' rows * vert's cols ("col-major").
	//Or just call with GL_TRUE in glUniformMatrix4fv, transposes the 4x4's as they are moved from CPU-side to here on GPU-side.
	gl_Position = pos;
}