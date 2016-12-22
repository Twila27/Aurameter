#version 410 core

uniform sampler2D uTexDiffuse; //The FBO color render target.
uniform sampler2D uTexDepth; //The FBO depth-stencil render target.

in vec2 passUV0;

//You can add more out vec4 colors here so long as your FBO has multiple color render targets.
out vec4 outColor; 
	//The order they are declared == the order they are bound to the FBO's render targets.

void main()
{	
	vec4 diffuse = texture( uTexDiffuse, passUV0 );

	float depthHere = texture( uTexDepth, passUV0 ).r;
		//Depths are cleared to white by default when calling glClear.
	float scale = 1.0f - (depthHere * depthHere); //As we get farther away, we sample farther/closer to it.

	float outlineThickness = .02f;
	float depthOneToRight = texture( uTexDepth, passUV0 + scale * vec2( outlineThickness, 0.f ) ).r; //Could make skinnier by shrinking the .05f's here and in next line.
	float depthOneToLeft = texture( uTexDepth, passUV0 - scale * vec2( outlineThickness, 0.f ) ).r; 
	float depthOneAbove = texture( uTexDepth, passUV0 + scale * vec2( 0.f, outlineThickness ) ).r; 
	float depthOneBelow = texture( uTexDepth, passUV0 - scale * vec2( 0.f, outlineThickness ) ).r; 
	
	depthOneToLeft *= depthOneToLeft; //"Depth correction"?
	depthOneToRight *= depthOneToRight;
	depthOneAbove *= depthOneAbove;
	depthOneBelow *= depthOneBelow;
	
	float diffHoriz = abs( depthOneToRight - depthOneToLeft ); //Could mult by how much you want and floor it to go branchless.
	float diffVertical = abs( depthOneAbove - depthOneBelow );
	
	const float edgeDetectThreshold = .15f;
	const vec4 outlineColor = vec4(0,0,0,1);
	if ( diffHoriz > edgeDetectThreshold || diffVertical > edgeDetectThreshold )
		outColor = outlineColor;
	else
		outColor = vec4(1);
	
		
	outColor *= diffuse;
}