#version 410 core

#define CYAN
#undef MAGENTA
#undef YELLOW
#undef BLACK

uniform sampler2D uTexDiffuse;

in vec2 passUV0;

//You can add more out vec4 colors here so long as your FBO has multiple color render targets.
out vec4 outColor; 
	//The order they are declared == the order they are bound to the FBO's render targets.

	
//CMYK: see other fragment shaders for separations, this collects them all but uses preprocessor above.
void main()
{
	vec4 diffuse = texture( uTexDiffuse, passUV0 );
	
	vec3 colorCMY = vec3( 1. ) - diffuse.rgb; //Convert RGB to CMY by subtracting RGB color from white.
		//RGB model is emissive, i.e. adding up onto black. But CMY model is transmissive, subtracting down from white.
	float colorK = min( colorCMY.x, min( colorCMY.y, colorCMY.z ) ); //Find amount of black in each color...
	colorCMY = colorCMY - ( .1 * colorK ); //and subtract K free of CMY. Same as ( colorCMY - vec3(colorK) ) / ( 1. - colorK )?
		// colorCMY/(1-colorK) - vec3(1.) == [ colorCMY - vec3( 1-colorK ) ] / (1-colorK) ???
		
	//Now adjust K slightly to adjust conversion to CMYK to more accurately resemble expected results.
	const float S_K = 0.1;
	const float K_0 = 0.3;
	const float oneMinusK_0 = 1.0 - K_0;
	const float K_max = 0.9;
	
	if ( colorK < K_0 ) colorK = 0.0;
	else colorK = K_max * ( colorK - K_0 ) / oneMinusK_0;
	vec4 colorCMYK = vec4( colorCMY, colorK );
	
	outColor.a = 1.0;
	
	#ifdef CYAN
	outColor.rgb = vec3( 1. - colorCMYK.x );
	#endif
	
	#ifdef MAGENTA
	outColor.rgb = vec3( 1. - colorCMYK.y );
	#endif
	
	#ifdef YELLOW
	outColor.rgb = vec3( 1. - colorCMYK.z );
	#endif
	
	#ifdef BLACK
	outColor.rgb = vec3( 1. - colorCMYK.w );
	#endif
}