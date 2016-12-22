#version 410 core

uniform sampler2D uLeftAnaglyphTex; 
uniform sampler2D uRightAnaglyphTex;

//uniform sampler2D uTexDiffuse; 
	//Using the FBO's color target (rendered scene) instead of the normal two textures you'd pass in as above uniforms.
	//But there's no point in doing below interlacing if the check amounts to the same texture sample!
	
uniform vec2 uOffset;
uniform vec4 uColor;

in vec2 passUV0;

//You can add more out vec4 colors here so long as your FBO has multiple color render targets.
out vec4 outColor; 
	//The order they are declared == the order they are bound to the FBO's render targets.

//3D TV: "SmoothPicture" interlacing left and right images to transmit a single stereo image. TV then refreshes at 120 Hz left-right-left-right-etc.
void main()
{	
	int row = int( gl_FragCoord.y ); //Screen-space position.
	int col = int( gl_FragCoord.x );
	int sum = row + col; //Of the interlacing checkboard of fragments.
	
	vec4 color;
	if ( (sum % 2) == 0 )
		color = texture( uLeftAnaglyphTex, passUV0 );
	else
		color = texture( uRightAnaglyphTex, passUV0 );

	outColor = vec4( color, 1.0 );
}
