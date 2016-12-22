#pragma once

#if !defined( __TOOLS_FBX__ )

	#define __TOOLS_FBX__

//#includes go here
#include <vector>
#include "Engine/Math/Matrix4x4.hpp"
class Command;
class MeshBuilder;
class Skeleton;
class AnimationSequence;

struct SkinWeight
{
	//For every control point, 
	//keep the 4 largest weights in it--
	//this is our running largest/max collection of weights seen so far.
	Vector4<unsigned int> jointIndices;
	Vector4f boneWeights;
};

class ImportedScene
{
public:
	~ImportedScene();

	std::vector<MeshBuilder*> m_meshBuilders;
	std::vector<Skeleton*> m_skeletons;
	std::vector<AnimationSequence*> m_animationSequences;
};


void FBXListSceneFromFile( const char* filename ); //Prints to a file or the output log.
void FBXList( Command& );
ImportedScene* FBXLoadSceneFromFile( const char* filename, const Matrix4x4f& engineBasis, bool isEngineBasisRightHanded, const Matrix4x4f& transform = Matrix4x4f::IDENTITY, float samplingFramerate = 30.f, bool useLocalOverGlobal = false );
void FBXLoad( Command& );
void MeshSaveLastMeshBuilderMade( Command& args );
void SkeletonSaveLastSkeletonMade( Command& args );
void AnimationSaveLastAnimationMade( Command& args );

#endif


void MeshLoadFromFile( Command& args );
void SkeletonLoadFromFile( Command& args );
void AnimationLoadFromFile( Command& args );