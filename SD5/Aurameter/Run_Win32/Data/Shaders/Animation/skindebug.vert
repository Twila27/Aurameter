#version 410 core

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

//Uniform Blocks are SUPER USEFUL for this.
#define MAX_SUPPORTED_BONES 200 //i.e. total in a skeleton.
uniform mat4 uBoneMatrices[ MAX_SUPPORTED_BONES ]; // inverseOfInitialJointTransform * currentInAnimationTransformOfJoint.

in vec3 inPosition;
in vec4 inColor;
in vec2 inUV0;
in vec2 inUV1;
in vec2 inUV2;
in vec2 inUV3;
in vec3 inTangent;
in vec3 inBitangent;
in vec3 inNormal;
//... everything else as usual

//NEW BUT HEED WARNINGS
in uvec4 inBoneIndices; //uints. When you pass this up, it is not glVertexAttribPointer, which converts to a float, but glVertexAttribIPointer to pass up integral values.
in vec4 inBoneWeights; //Needs to already be normalized so the components guaranteed to add up to 1.

out vec2 passUV0;
out vec2 passUV1;
out vec2 passUV2;
out vec2 passUV3;
out vec3 passWorldSpacePosition;
out vec4 passthroughColor;
out vec3 passNormal;
out vec3 passTangent;
out vec3 passBitangent;
// out ... // everything usually passed out

void main( void )
{
	//Grab the 4 bones we care about--we do not care about other contributions in the bone weights' weighted sum.
	mat4 bone0 = uBoneMatrices[ inBoneIndices.x ];
	mat4 bone1 = uBoneMatrices[ inBoneIndices.y ];
	mat4 bone2 = uBoneMatrices[ inBoneIndices.z ];
	mat4 bone3 = uBoneMatrices[ inBoneIndices.w ];
	
	//!\ Assuming inBoneWeights is already normalized!
	mat4 finalBoneTransform = inBoneWeights.x * bone0
		+ inBoneWeights.y * bone1
		+ inBoneWeights.z * bone2
		+ inBoneWeights.w * bone3; //Weighted average from initial model to current-in-animation model space. But this is still in model space, need to go to world space.
		
	//For debug purposes, you can also do mat4 modelToWorld = uModel; which will remove the skinning calculations.
	//So you can still output their colors without actually applying the skinning to change vertex positions.
	mat4 modelToWorld = /*finalBoneTransform **/ uModel; //THIS IS FOR ROW-major.
		//normal: blinnphong.vert and vdefn pct 
		//goal: this.vert and vdefn super --works w/above finalBoneTransform excluded
	
	//Since this is the debug shader, we want to set our passColor to our boneweight (ignoring last one else it'd be transparent):
	passthroughColor = vec4( inBoneWeights.xyz, 1.f ); //In real version, just assign inColor instead.
	passUV0 = inUV0;
	passUV1 = inUV1;
	passUV2 = inUV2;
	passUV3 = inUV3;
	
	//note index 77 on unity-chan is the joint "Character1_Head" 
	//if ( inBoneIndices.x == 77 ) //was not hit, even though it looks like it's this in FBXUtils.cpp's importedScene's mb's...
	//	passthroughColor = vec4( 1, 0, 0, 1 );
	//else if ( inBoneIndices.z == 77 ) //was hit? how? z is 0 in the importedScene's mb's...
	//	passthroughColor = vec4( 1, 1, 0, 1 );
//	if ( inBoneWeights.xyz == vec3( 0, 0, 1 ) ) //was hit as well, proving the above
//		passthroughColor = vec4( 0, 0, 0, 1 );
		
	//Everything else in model space needs to be transformed as well.
	passWorldSpacePosition = ( vec4( inPosition, 1.f ) * modelToWorld ).xyz; //Again, watch out for ordering, match my previous ones.
	passTangent = (vec4(inTangent, 0.f) * modelToWorld).xyz;
	passBitangent = (vec4(inBitangent, 0.f) * modelToWorld).xyz;
	passNormal = (vec4(inNormal, 0.f) * modelToWorld).xyz;
	
	gl_Position = vec4(inPosition, 1.f) * modelToWorld * uView * uProj;
}