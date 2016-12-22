#version 410 core

uniform sampler2D uTexDiffuse; //The FBO color render target.
uniform sampler2D uTexDepth; //The FBO depth-stencil render target.

uniform float uWrappingTimer;
uniform float uWrappingTimerDuration;

in vec2 passUV0;

//You can add more out vec4 colors here so long as your FBO has multiple color render targets.
out vec4 outColor; 
	//The order they are declared == the order they are bound to the FBO's render targets.


//Embossing: modify edge detect by luminance to replace color by luminance and highlight images differently depending on edges' angles.
void main()
{	
	ivec2 resolution = textureSize( uTexDiffuse, 0 );
	float resolutionS = float( resolution.s );
	float resolutionT = float( resolution.t );
	vec4 diffuse = texture( uTexDiffuse, passUV0 );
	
	//Offsets / stride between texels.
	vec2 stepRight = vec2( 1.0/resolutionS, 0.0 ); //One rightward step.
	vec2 stepUpRight = vec2( 1.0/resolutionS, 1.0/resolutionT ); //One up-right step.
	vec4 diffuseAtStepUpRight = texture( uTexDiffuse, passUV0 + stepUpRight );
	
	vec4 colorDist = diffuse - diffuseAtStepUpRight;
	float max = colorDist.r;
	if ( abs(colorDist.g) > abs(max) ) max = colorDist.g;
	if ( abs(colorDist.b) > abs(max) ) max = colorDist.b;
	
	float gray = clamp( max + .5, 0.0, 1.0 );
	vec3 color = vec3( gray );
	outColor = vec4( color, 1.0 );
}