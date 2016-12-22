#version 410 core

uniform sampler2D uTexDiffuse; //The FBO color render target.
uniform sampler2D uTexDepth; //The FBO depth-stencil render target.

uniform float uWrappingTimer;
uniform float uWrappingTimerDuration;

in vec2 passUV0;

//You can add more out vec4 colors here so long as your FBO has multiple color render targets.
out vec4 outColor; 
	//The order they are declared == the order they are bound to the FBO's render targets.


//Edge Detection with Sobel Filter: unlike lecture which used depth buffer values, this uses differences between image texels' luminance values.
void main()
{	
	ivec2 resolution = textureSize( uTexDiffuse, 0 );
	float resolutionS = float( resolution.s );
	float resolutionT = float( resolution.t );
	vec4 diffuse = texture( uTexDiffuse, passUV0 );
	
	const vec3 luminanceWeightVector = vec3( .2125, .7154, .0721 );
	
	//Offsets / stride between texels.
	vec2 stepRight = vec2( 1.0/resolutionS, 0.0 ); //One rightward step.
	vec2 stepUp = vec2( 0.0, 1.0/resolutionT ); //One upward step.
	vec2 stepUpRight = vec2( 1.0/resolutionS, 1.0/resolutionT ); //One up-right step.
	vec2 stepDownRight = vec2( 1.0/resolutionS, -1.0/resolutionT ); //One lower-right step.
	
	//Inner 3x3 pixel colors next. Generate 25 colors from 13 UV coordinate pairs. Ideally break this into two 1D passes for less samples!
	float fragSampleLuminanceCenter = dot( texture( uTexDiffuse, passUV0 ).rgb, luminanceWeightVector );
	float fragSampleLuminanceDownOneLeftOne = dot( texture( uTexDiffuse, passUV0-stepUpRight ).rgb, luminanceWeightVector );
	float fragSampleLuminanceUpOneRightOne = dot( texture( uTexDiffuse, passUV0+stepUpRight ).rgb, luminanceWeightVector );
	float fragSampleLuminanceUpOneLeftOne = dot( texture( uTexDiffuse, passUV0-stepDownRight ).rgb, luminanceWeightVector );
	float fragSampleLuminanceDownOneRightOne = dot( texture( uTexDiffuse, passUV0+stepDownRight ).rgb, luminanceWeightVector );
	float fragSampleLuminanceLeftOne = dot( texture( uTexDiffuse, passUV0-stepRight ).rgb, luminanceWeightVector );
	float fragSampleLuminanceRightOne = dot( texture( uTexDiffuse, passUV0+stepRight ).rgb, luminanceWeightVector );
	float fragSampleLuminanceDownOne = dot( texture( uTexDiffuse, passUV0-stepUp ).rgb, luminanceWeightVector );
	float fragSampleLuminanceUpOne = dot( texture( uTexDiffuse, passUV0+stepUp ).rgb, luminanceWeightVector );
	
	float horizontalSobelValue = -1.0*fragSampleLuminanceUpOneLeftOne 
	+ -2.0*fragSampleLuminanceUpOne 
	+ -1.0*fragSampleLuminanceUpOneRightOne 
	+ 1.0*fragSampleLuminanceDownOneLeftOne 
	+ 2.0*fragSampleLuminanceDownOne
	+ 1.0*fragSampleLuminanceDownOneRightOne;
	
	float verticalSobelValue = -1.0*fragSampleLuminanceDownOneLeftOne
	+ -2.0*fragSampleLuminanceLeftOne
	+ -1.0*fragSampleLuminanceUpOneLeftOne
	+ 1.0*fragSampleLuminanceDownOneRightOne
	+ 2.0*fragSampleLuminanceRightOne
	+ 1.0*fragSampleLuminanceUpOneRightOne;
	
	float magnitude = length( vec2( horizontalSobelValue, verticalSobelValue ) ); //Combines the two Sobel filter results.
		//magnitude == "How much change/delta was there over all dimensions?"
	vec3 target = vec3( magnitude );
	float threshold = uWrappingTimer / uWrappingTimerDuration;
	outColor = vec4( mix( diffuse.rgb, target, threshold ), 1.0 );
}