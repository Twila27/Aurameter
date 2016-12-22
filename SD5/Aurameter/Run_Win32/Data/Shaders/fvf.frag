//Fixed-vertex format or fixed-format pipeline.

#version 410 core

uniform vec4 uColor;
uniform sampler2D uDiffuseTex;

//These are the attributes passed in from the vertex shader.
in vec3 inPosition;
in vec4 passthroughColor; //Must share names with .vert to get match.
in vec2 passUV0; //0 in case we want to add more pairs of UVs later.

out vec4 finalColor; //The final output of frag shader.


void main( void )
{	
//	finalColor = passthroughColor * vec4( sin( gl_FragCoord.x ), cos( gl_FragCoord.y ) * .4f, 0.f, 1.f ); //Says to output a green hue for this fragment.
	
	finalColor = uColor * passthroughColor; //vec4(1.0f);
	
	vec4 diffuseTexColor = texture(uDiffuseTex, passUV0);
	//if ( passUV0.x == 0.0f ) finalColor = vec4(0.5f);
	//finalColor = vec4( passUV0, 0.0f, 1.0f ); //Debug test.
		//Outputs the UV == RG channel.
	finalColor = passthroughColor * uColor * diffuseTexColor;
		//uColor currently that cornflower blue.
		//passthroughColor being set to inColor, which is different per corner to get a rainbow effect.
}