#version 410 core

uniform sampler2D uTexDiffuse; //Used as foreground image.
uniform sampler2D uTexDiffuseChromakeyBG;
uniform float uWrappingTimer;
uniform float uWrappingTimerDuration;

in vec2 passUV0;

//You can add more out vec4 colors here so long as your FBO has multiple color render targets.
out vec4 outColor; 
	//The order they are declared == the order they are bound to the FBO's render targets.

	
//Chromakey: within a typically very small tolerance (uWrappingTimer) replace certain colors with the background texture.
void main()
{
	vec3 diffuseFG = texture( uTexDiffuse, passUV0 ).rgb;
	vec3 diffuseBG = texture( uTexDiffuseChromakeyBG, passUV0 ).rgb;
	vec4 color;
	
	float r = diffuseFG.r;
	float g = diffuseFG.g;
	float b = diffuseFG.b;
	
	color = vec4( diffuseBG, 1.0 );
	float tolerance = uWrappingTimer / uWrappingTimerDuration;
	float limitR = tolerance;
	float limitG = 1.0 - tolerance; // One-minus-tolerance: put this here to replace colors near green, next line for colors near blue, etc.
	float limitB = tolerance;
	
	if ( r <= limitR && g >= limitG && b <= limitB ) // If the fragment color is within tolerance of (r,g,b).
		color = vec4( diffuseBG, 1.0 );
	else
		color = vec4( diffuseFG, 1.0 ); //Could use mix( diffuseFG, diffuseBG, uAlpha ) but going to keep it to one sliding timer for now. 
		
	outColor = color;
}