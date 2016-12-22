#version 410 core

//Recall, uniforms are 'uniform' across all vertices, where in-variables are unique per-vertex. 
uniform mat4 uProj; //Will end up being our ortho, but still a proj matrix. 
uniform mat4 uView; //NO uModel because the sprites will be in their world positions already when we make the meshes. 
					//Since we're making these meshes each frame in world space.
					//Very typical for a 2D sprite system or particle system.

in vec2 inPosition;
in vec2 inUV0;
in vec4 inColor;

out vec2 passthroughUV0;
out vec4 passthroughTintColor;

void main()
{
	passthroughUV0 = inUV0;
	passthroughTintColor = inColor;

	vec4 pos = vec4( inPosition, 0, 1 ); //1 to preserve translation, 0 to ignore depth.
										 //Could sort by using a value other than 0 for z, rather than sorting by layers.
										 //This would use the depth buffer, but encounters issues of z-fighting, i.e. layers flickering back and forth, whereas painters always just splats one onto the other.
	gl_Position = pos * uView * uProj;
}