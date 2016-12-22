#pragma once


#include <vector>
#include <string>
#include "Engine/Math/Matrix4x4.hpp"
#include "Engine/Renderer/Rgba.hpp"


//-----------------------------------------------------------------------------
#define INVALID_JOINT_INDEX -1


//-----------------------------------------------------------------------------
class Skeleton;
extern Skeleton* g_lastLoadedSkeleton; //Used for saving out skeletons.


//-----------------------------------------------------------------------------
class BinaryWriter;
class BinaryReader;


//-----------------------------------------------------------------------------
struct SkeletonJoint
{
	SkeletonJoint( uint32_t parallelArraysIndex ) : m_parallelArraysIndex( parallelArraysIndex ), m_parent( nullptr ) {}
	uint32_t m_parallelArraysIndex;
	SkeletonJoint* m_parent;
	std::vector<SkeletonJoint*> m_children;
	Matrix4x4f m_localJointSpaceTransform;
};


//-----------------------------------------------------------------------------
class Skeleton
{
public: 
	~Skeleton();

	//Currently storing the SkeletonJoints as parallel arrays of their data, because it'll be necessary for weights/serialization.
	std::vector<std::string> m_jointNameIDs;
	std::vector<int32_t> m_indicesOfParentJoints;
	std::vector<Matrix4x4f> m_unchangingGlobalJointModelToBoneSpaceTransforms; //NEVER changes after import.
	std::vector<Matrix4x4f> m_globalJointBoneToModelSpaceTransforms; //Acted upon by setters.
	//Therefore, whatever index you use to access these arrays == the index identifying the current joint.

	void GetBoneMatrices( Matrix4x4f* out_matrices, unsigned int numMatrices );
	int GetNumJoints() const { return m_joints.size(); }
	int GetLastAddedJointIndex() const;
	int GetJointIndexForName( const std::string& name ) const;
	SkeletonJoint* GetJointForIndex( unsigned int index ) const;
	void SetLocalJointSpaceTransformForJointIndex( int jointIndex, Matrix4x4f& newTransformInLocalJointSpace );
	void SetGlobalModelSpaceTransformForJointIndex( int jointIndex, Matrix4x4f& newTransformInGlobalModelSpace ); //Only set bone-to-model!
	void AddJoint( const char* jointName, int parentJointIndex, Matrix4x4f& globalModelSpaceTransform );

	bool WriteToFile( const char* filename, bool appendToFile, int endianMode );
	bool WriteToStream( BinaryWriter& writer );
	bool ReadFromFile( const char* filename, int endianMode );
	bool ReadFromStream( BinaryReader& reader );

	void VisualizeSkeleton( const Rgba& boneColor = Rgba::CYAN, float boneThickness = 1.f, const Rgba& jointColor = Rgba::MAGENTA, float jointSizeScalar = .001f );
	void RecursivelyDrawBones( SkeletonJoint* currentJoint, const Rgba& boneColor, float boneThickness );

	//Tree transform hierarchy representation: needed for IK, ragdolls, and bone masks (like the UE4 pistol + walk blend).		
	SkeletonJoint* m_hierarchyRoot;
	std::vector<SkeletonJoint*> m_joints;

private:	
	static const uint32_t s_FILE_VERSION = 1;

	
	bool ReadOldVersionFormatFromStream( BinaryReader& reader );
	void ReconstructLocalTransformHierarchy();
};

/* Skeleton Serialization Format v1.0 (AES A04)
	1. FILE VERSION -- this way we don't try to load an old format, just forbid it.
	2. # Joints
	3. String Names[#joints]
	4. Parent Indices[#joints]
	5. Joint-Bone to Model Space Transforms[#joints]
*/