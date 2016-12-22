//Fixed-vertex format or fixed-format pipeline.

#version 410 core

uniform int uUseTime;
uniform float uWrappingTimer;
uniform sampler2D uDiffuseTex;

//These are the attributes passed in from the vertex shader.
in vec3 inPosition;
in vec4 passthroughColor; //Must share names with .vert to get match.
in vec2 passUV0; //0 in case we want to add more pairs of UVs later.

out vec4 finalColor; //The final output of frag shader.


void main( void )
{	
//	finalColor = passthroughColor * vec4( sin( gl_FragCoord.x ), cos( gl_FragCoord.y ) * .4f, 0.f, 1.f ); //Says to output a green hue for this fragment.
	
	finalColor = passthroughColor; //vec4(1.0f);
	
	vec2 scrolledUV = passUV0;
	if ( uUseTime > 0 ) 
		scrolledUV = scrolledUV * vec2( uWrappingTimer ); //WARNING: will go over 1.0, what will happen then?????????????????
	
	vec4 diffuseTexColor = texture(uDiffuseTex, scrolledUV);
	//if ( passUV0.x == 0.0f ) finalColor = vec4(0.5f);
	//finalColor = vec4( passUV0, 0.0f, 1.0f ); //Debug test.
		//Outputs the UV == RG channel.
	finalColor = passthroughColor * diffuseTexColor; 
	if ( uUseTime > 0 ) 
		finalColor = finalColor * vec4(vec3(uWrappingTimer), 1.0f);
		//uColor currently that cornflower blue.
		//passthroughColor being set to inColor, which is different per corner to get a rainbow effect.
}