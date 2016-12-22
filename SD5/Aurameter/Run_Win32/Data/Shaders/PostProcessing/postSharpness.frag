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
vec3 ApplyGaussianBlurFilter()
{
	vec2 resolution = textureSize( uTexDiffuse, 0 );
	float resolutionS = float( resolution.s );
	float resolutionT = float( resolution.t );
	vec4 diffuse = texture( uTexDiffuse, passUV0 );
	
	//Offsets / stride between texels.
	vec2 stepRight = vec2( 1.0/resolutionS, 0.0 ); //One rightward step.
	vec2 stepUp = vec2( 0.0, 1.0/resolutionT ); //One upward step.
	vec2 stepUpRight = vec2( 1.0/resolutionS, 1.0/resolutionT ); //One up-right step.
	vec2 stepDownRight = vec2( 1.0/resolutionS, -1.0/resolutionT ); //One lower-right step.
	
	//Inner 3x3 pixel colors next. Generate 25 colors from 13 UV coordinate pairs. Ideally break this into two 1D passes for less samples!
	vec3 fragSampleCenter = texture( uTexDiffuse, passUV0 ).rgb;
	vec3 fragSampleDownOneLeftOne = texture( uTexDiffuse, passUV0-stepUpRight ).rgb;
	vec3 fragSampleUpOneRightOne = texture( uTexDiffuse, passUV0+stepUpRight ).rgb;
	vec3 fragSampleUpOneLeftOne = texture( uTexDiffuse, passUV0-stepDownRight ).rgb;
	vec3 fragSampleDownOneRightOne = texture( uTexDiffuse, passUV0+stepDownRight ).rgb;
	vec3 fragSampleLeftOne = texture( uTexDiffuse, passUV0-stepRight ).rgb;
	vec3 fragSampleRightOne = texture( uTexDiffuse, passUV0+stepRight ).rgb;
	vec3 fragSampleDownOne = texture( uTexDiffuse, passUV0-stepUp ).rgb;
	vec3 fragSampleUpOne = texture( uTexDiffuse, passUV0+stepUp ).rgb;
	
	//Outer ring of the 5x5.
	vec3 fragSampleDownTwoLeftTwo = texture( uTexDiffuse, passUV0-(2*stepUpRight) ).rgb;
	vec3 fragSampleUpTwoRightTwo = texture( uTexDiffuse, passUV0+(2*stepUpRight) ).rgb;
	vec3 fragSampleUpTwoLeftTwo = texture( uTexDiffuse, passUV0-(2*stepDownRight) ).rgb;
	vec3 fragSampleDownTwoRightTwo = texture( uTexDiffuse, passUV0+(2*stepDownRight) ).rgb;
	
	vec3 fragSampleUpTwoLeftOne = texture( uTexDiffuse, passUV0+(stepUp-stepDownRight) ).rgb;
	vec3 fragSampleUpTwoRightOne = texture( uTexDiffuse, passUV0+(stepUp+stepUpRight) ).rgb;
	vec3 fragSampleDownTwoLeftOne = texture( uTexDiffuse, passUV0-(stepUp+stepUpRight) ).rgb;
	vec3 fragSampleDownTwoRightOne = texture( uTexDiffuse, passUV0-(stepUp-stepDownRight) ).rgb;
	
	vec3 fragSampleUpOneLeftTwo = texture( uTexDiffuse, passUV0-(stepDownRight+stepRight) ).rgb;
	vec3 fragSampleUpOneRightTwo = texture( uTexDiffuse, passUV0+(stepUpRight+stepRight) ).rgb;
	vec3 fragSampleDownOneLeftTwo = texture( uTexDiffuse, passUV0-(stepUpRight+stepRight) ).rgb;
	vec3 fragSampleDownOneRightTwo  = texture( uTexDiffuse, passUV0+(stepDownRight+stepRight) ).rgb;
	
	vec3 fragSampleLeftTwo = texture( uTexDiffuse, passUV0-(2*stepRight) ).rgb;
	vec3 fragSampleRightTwo = texture( uTexDiffuse, passUV0+(2*stepRight) ).rgb;
	vec3 fragSampleDownTwo = texture( uTexDiffuse, passUV0-(2*stepUp) ).rgb;
	vec3 fragSampleUpTwo = texture( uTexDiffuse, passUV0+(2*stepUp) ).rgb;
	
	//ADD THE MATRIX FORM OF THE BELOW!
	vec3 color = vec3( 0.0 );
	color += 1.0 * ( fragSampleDownTwoLeftTwo + fragSampleDownTwoRightTwo + fragSampleUpTwoLeftTwo + fragSampleUpTwoRightTwo );
	color += 4.0 * ( fragSampleUpTwoLeftOne + fragSampleUpTwoRightOne + fragSampleDownTwoLeftOne + fragSampleDownTwoRightOne );
	color += 7.0 * ( fragSampleRightTwo + fragSampleLeftTwo + fragSampleUpTwo + fragSampleDownTwo );
	color += 16.0 * ( fragSampleUpOneLeftOne + fragSampleUpOneRightOne + fragSampleDownOneLeftOne + fragSampleDownOneRightOne );
	color += 26.0 * ( fragSampleRightOne + fragSampleLeftOne + fragSampleUpOne + fragSampleDownOne );
	color += 41.0 * ( fragSampleCenter );
	color /= 273; //Sum of the weights.
	return color;
}

void main()
{	
	vec3 diffuse = texture( uTexDiffuse, passUV0 ).rgb;	
	vec3 blurredDiffuse = ApplyGaussianBlurFilter();
	vec3 sharpenedDiffuse = mix( diffuse, blurredDiffuse, uWrappingTimer ); //If t > 1, we anti-blur == sharpen the image!
	outColor = vec4( sharpenedDiffuse.rgb, 1.0 );
}