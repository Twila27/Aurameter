#version 410 core

uniform vec4 uColor;

//These are the attributes passed in from the vertex shader.
in vec2 passUV0;
in vec3 inPosition;
in vec4 passthroughColor; //Must share names with .vert to get match.

out vec4 finalColor; //The final output of frag shader.

vec4 ColorFromVector( vec3 v ) //Map a normalized 0-1 vector to RGB colorspace.
{
	return vec4((v + vec3(1.0f)) * .5f, 1.0f); 
		//Convert a unit vector into color space.
		//This lets us validate our vector values.
		//In this system, a hue of 0 == -1, 127 == 0, 255 == +1.
}


vec4 CalcMandelbrotValue( void )
{
	float real = passUV0.s;
	float imag = passUV0.t;
	float real0 = real;
	float imag0 = imag;
	float newr;	
	int numIters;
	int maxIters = 1000;
	vec4 color = vec4( vec3(0.), 1. );
	
	float uLimit = .72f;
	vec3 uConvergeColor = vec3(1., 0., 0.);
	vec3 uDivergeColor1 = vec3(0., 1., 0.);
	vec3 uDivergeColor2 = vec3(0., 0., 1.);
	
	for ( numIters = 0; numIters < maxIters; numIters++ )
	{
		float newreal = real0 + real*real - imag*imag;
		float newimag = imag0 + 2.*real*imag;
		newr = newreal*newreal + newimag*newimag;
		if ( newr >= uLimit )
			break;
		real = newreal;
		imag = newimag;
	}
	
	//choose the color
	if ( newr < uLimit )
		color.rgb = uConvergeColor;
	else
		color.rgb = mix( uDivergeColor1, uDivergeColor2, numIters );
			
//		color.rgb *= vLightIntensity;
	
	return color;
}	


void main( void )
{	
//	finalColor = passthroughColor * vec4( sin( gl_FragCoord.x ), cos( gl_FragCoord.y ) * .4f, 0.f, 1.f ); //Says to output a green hue for this fragment.
	
	finalColor = CalcMandelbrotValue(); //vec4(1.0f);
}