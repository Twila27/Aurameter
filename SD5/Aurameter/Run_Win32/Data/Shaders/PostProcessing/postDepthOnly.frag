#version 410 core

uniform sampler2D uTexDepth; //The FBO depth-stencil render target.

in vec2 passUV0;

//You can add more out vec4 colors here so long as your FBO has multiple color render targets.
out vec4 outColor; 
	//The order they are declared == the order they are bound to the FBO's render targets.

void main()
{
	float depth = texture( uTexDepth, passUV0 ).r; //First channel is depth (24 bits of precision for one float value), second is stencil.
		//Please note the faintness depends greatly on the difference of zNear and zFar, and esp. increasing zNear will make things more black / toward zero.
		//White generally appears more due to perspective projection making a nonlinear falloff, as it causes double-division by z (i.e. mult by z^-2 ).
		//Depths are cleared to white by default when calling glClear.
	depth *= depth;
	outColor = vec4( vec3(depth), 1 );
}