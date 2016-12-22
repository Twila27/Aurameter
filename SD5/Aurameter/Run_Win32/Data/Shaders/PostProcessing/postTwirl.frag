#version 410 core

uniform sampler2D uTexDiffuse;
uniform float uUnwrappedTimer;
uniform float uWrappingTimer;
uniform float uWrappingTimerDuration;

in vec2 passUV0;

//You can add more out vec4 colors here so long as your FBO has multiple color render targets.
out vec4 outColor; 
	//The order they are declared == the order they are bound to the FBO's render targets.

const float PI = 3.14159265;
	
void main()
{	
	float resolution = float( textureSize( uTexDiffuse, 0 ).s ); //assume it's a square texture

	vec2 uv = passUV0;
	float radius = resolution * uWrappingTimer; //WRAPPER
	vec2 xy = resolution * uv; //frag coords from tex coords
	
	vec2 dxy = xy - 0.5*resolution; // twirl center is ( res/2, res/2 )
	float r = length( dxy );
	float beta = atan( dxy.y, dxy.x ) + ( radians( uWrappingTimerDuration )*(radius-r)/radius );
	
	vec2 xy1 = xy;
	if ( r <= radius )
	{
		xy1 = 0.5*resolution + r*vec2( cos(beta), sin(beta) ); //Defines circle.
	}
	uv = xy1/resolution; //restoring coordinates
	
	vec3 diffuse = texture( uTexDiffuse, uv ).rgb;
	outColor = vec4( diffuse, 1.0 );
}