#version 410 core

//uniform sampler2D uLeftAnaglyphTex; uniform sampler2D uRightAnaglyphTex;
uniform sampler2D uTexDiffuse; //Using the FBO's color target (rendered scene) instead of the normal two textures you'd pass in as above uniforms.
uniform vec2 uOffset;
uniform vec4 uColor;

in vec2 passUV0;

//You can add more out vec4 colors here so long as your FBO has multiple color render targets.
out vec4 outColor; 
	//The order they are declared == the order they are bound to the FBO's render targets.

//Anaglyph: 3D glasses red-cyan (R, G+B).
void main()
{	
	vec4 left = texture( uTexDiffuse, passUV0 );
	vec4 right = texture( uTexDiffuse, passUV0 + vec2(uOffset.s, uOffset.t) ); //Additional offset control.
	
	vec3 color = vec3( left.r, right.gb );
	color *= vec3( uColor.r, uColor.g, uColor.b ); //Additional scale control, otherwise just scale by white.
	color = clamp( color, 0.0, 1.0 );
	
	outColor = vec4( color, 1.0 );
}
