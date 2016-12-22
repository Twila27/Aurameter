#version 410 core

uniform sampler2D uTexDiffuse;
uniform float uUnwrappedTimer;
uniform vec4 uColor;

in vec2 passUV0;

//You can add more out vec4 colors here so long as your FBO has multiple color render targets.
out vec4 outColor; 
	//The order they are declared == the order they are bound to the FBO's render targets.

void main()
{
	vec2 rippleCenter = vec2(-.5f); //In normalized device coordinates. Originally was .5f, had to negate to move it to center.
	vec2 dispFromCenter = normalize( passUV0 - rippleCenter );
	
	//Use this for jelly/sway: float offsetAmount = .05f * -cos( (uUnwrappedTimer * 4.f) + length(dispFromCenter)*100.f );
	float offsetAmount = .05f * -cos( (uUnwrappedTimer * 4.f) + length(passUV0 - rippleCenter)*100.f ); //Tweak a cosine wave by time to offset the texture sampler's caret.
	vec2 uv = passUV0 + dispFromCenter * offsetAmount;

	outColor = uColor * texture(uTexDiffuse, uv);
}