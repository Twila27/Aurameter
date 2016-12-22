#version 410 core

//A layer tint to tint the entire layer could be sent as a uniform, e.g. fade out the UI and game layers on pause.
//Noted that layer effects would all be shaders.
uniform sampler2D uTexDiffuse;

in vec2 passthroughUV0;
in vec4 passthroughTintColor;

out vec4 outColor;

vec4 CalcMandelbrotValue( void )
{
	float real = passthroughUV0.s;
	float imag = passthroughUV0.t;
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
	outColor = passthroughTintColor * vec4( sin( gl_FragCoord.x ), cos( gl_FragCoord.y ) * .4f, 1.f, 1.f ); //Says to output a green hue for this fragment.
	
	vec4 diffuseColor = texture( uTexDiffuse, passthroughUV0 );
	outColor *= diffuseColor * CalcMandelbrotValue(); //vec4(1.0f);
}