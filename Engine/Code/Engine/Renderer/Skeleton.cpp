#include "Engine/Renderer/Skeleton.hpp"
#include "Engine/FileUtils/Writers/FileBinaryWriter.hpp"
#include "Engine/FileUtils/Readers/FileBinaryReader.hpp"
#include "Game/GameCommon.hpp"


//--------------------------------------------------------------------------------------------------------------
Skeleton* g_lastLoadedSkeleton = nullptr;


//--------------------------------------------------------------------------------------------------------------
Skeleton::~Skeleton()
{
	for ( SkeletonJoint* joint : m_joints )
		if ( joint != nullptr )
			delete joint;
}


//--------------------------------------------------------------------------------------------------------------
void Skeleton::GetBoneMatrices( Matrix4x4f* out_matrices, unsigned int numMatrices )
{
	const unsigned int jointCount = GetNumJoints();
	ASSERT_OR_DIE( numMatrices >= jointCount, "Skeleton::GetBoneMatrices() - Not enough room in out_matrices!" );

	for ( unsigned int jointIndex = 0; jointIndex < jointCount; jointIndex++ )
	{
		Matrix4x4f modelToInitialBoneMatrix = m_unchangingGlobalJointModelToBoneSpaceTransforms[ jointIndex ];
		Matrix4x4f currentModelMatrix = m_globalJointBoneToModelSpaceTransforms[ jointIndex ];
		out_matrices[ jointIndex ] = modelToInitialBoneMatrix * currentModelMatrix;
	}
}


//--------------------------------------------------------------------------------------------------------------
int Skeleton::GetLastAddedJointIndex() const
{
	//We just want the index, no need to return back().
	//Every time we add a joint, all 4 Skeleton class vectors get updated in parallel, so any member vector works:
	return m_jointNameIDs.size() - 1;
}


//--------------------------------------------------------------------------------------------------------------
int Skeleton::GetJointIndexForName( const std::string& name ) const
{
	for ( unsigned int jointIndex = 0; jointIndex < m_jointNameIDs.size(); jointIndex++ )
		if ( m_jointNameIDs[ jointIndex ] == name )
			return jointIndex;

	return -1;
}


//--------------------------------------------------------------------------------------------------------------
SkeletonJoint* Skeleton::GetJointForIndex( unsigned int index ) const
{
	if ( index < m_joints.size() )
		return m_joints[ index ];
	else
		return nullptr;
}


//--------------------------------------------------------------------------------------------------------------
void Skeleton::SetLocalJointSpaceTransformForJointIndex( int jointIndex, Matrix4x4f& newTransformInLocalJointSpace )
{
	//Tree hierarchy is already in local space.
	m_joints[ jointIndex ]->m_localJointSpaceTransform = newTransformInLocalJointSpace;

	//Parallel arrays are not, so we must convert:
	// Logically we have myLocal = myParentG^-1 * myGlobal, to solve myG we left-multiply both sides by myPG.
	// This gives myParentG * myL = myG.
	if ( jointIndex == 0 ) //Root case, no parent to worry about.
	{
		m_globalJointBoneToModelSpaceTransforms[ jointIndex ] = newTransformInLocalJointSpace;
	}
	else
	{
		int parentIndex = m_indicesOfParentJoints[ jointIndex ];
		Matrix4x4f parentGlobalTransform = m_globalJointBoneToModelSpaceTransforms[ parentIndex ];
		m_globalJointBoneToModelSpaceTransforms[ jointIndex ] = parentGlobalTransform * newTransformInLocalJointSpace; //!\ ORDERING MAY BE OFF!
	}
}


//--------------------------------------------------------------------------------------------------------------
void Skeleton::SetGlobalModelSpaceTransformForJointIndex( int jointIndex, Matrix4x4f& newTransformInGlobalModelSpace )
{
	//Parallel arrays are already in global space.
	m_globalJointBoneToModelSpaceTransforms[ jointIndex ] = newTransformInGlobalModelSpace;

	//Tree hierarchy is not, so we must convert: myLocal := (myParentGlobal^-1) * myGlobal, to cancel all but this joint's transform.
	if ( jointIndex == 0 ) //Root case, no parent to worry about.		
	{
		m_joints[ jointIndex ]->m_localJointSpaceTransform = newTransformInGlobalModelSpace;
	}
	else
	{
		int parentIndex = m_indicesOfParentJoints[ jointIndex ];
		Matrix4x4f parentGlobalTransform = m_globalJointBoneToModelSpaceTransforms[ parentIndex ];
		parentGlobalTransform.GetInverseAssumingOrthonormality( parentGlobalTransform );
		m_joints[ jointIndex ]->m_localJointSpaceTransform = newTransformInGlobalModelSpace * parentGlobalTransform; //!\ ORDERING MAY BE OFF!
	}
}


//--------------------------------------------------------------------------------------------------------------
void Skeleton::AddJoint( const char* jointName, int parentJointIndex, Matrix4x4f& globalTransform )
{
	//Create the joint.
	int newJointIndex = m_joints.size();
	SkeletonJoint* newJoint = new SkeletonJoint( newJointIndex );
	m_joints.push_back( newJoint );

	//Update the parallel arrays of information about all joints.
	m_jointNameIDs.push_back( jointName );
	m_indicesOfParentJoints.push_back( parentJointIndex );

	//Actual setting of the transform and updating of the tree.
	m_globalJointBoneToModelSpaceTransforms.push_back( Matrix4x4f::IDENTITY ); //Make room for new global model-space transform.
	SetGlobalModelSpaceTransformForJointIndex( newJointIndex, globalTransform ); //Also sets local joint-space transform for me.
		//Big error in Skinning assignment A6 was pushing the EvaluatedLocalTransform() into the container of global ones.
		//The skeleton/animation only looked right because we recalculate the global position from all locals every frame.

	//Use the global transform just pushed by above setter to get the reverse mapping, only used in animation.
	Matrix4x4f inverseTransform( m_globalJointBoneToModelSpaceTransforms.back().GetOrdering() );
	m_globalJointBoneToModelSpaceTransforms.back().GetInverseAssumingOrthonormality( inverseTransform );
	m_unchangingGlobalJointModelToBoneSpaceTransforms.push_back( inverseTransform );

	//Update the parent in the tree representation:
	if ( newJointIndex != 0 ) //Root node case.
	{
		SkeletonJoint* newJointParent = m_joints[ parentJointIndex ];
		newJointParent->m_children.push_back( newJoint );
		newJoint->m_parent = newJointParent;
	}
	else m_hierarchyRoot = newJoint;
}


//--------------------------------------------------------------------------------------------------------------
bool Skeleton::WriteToFile( const char* filename, bool appendToFile, int endianMode )
{
	bool didWrite = false;

	FileBinaryWriter writer;
	writer.SetEndianMode( (EndianMode)endianMode );

	didWrite = writer.open( filename, appendToFile );
	if ( !didWrite )
	{
		DebuggerPrintf( "File not accessible in Skeleton::WriteToFile!" );
		return false;
	}

	didWrite = this->WriteToStream( writer );
	if ( !didWrite )
	{
		DebuggerPrintf( "WriteToStream failed in Skeleton::WriteToFile!" );
		return false;
	}
	writer.close();

	return didWrite;
}


//--------------------------------------------------------------------------------------------------------------
bool Skeleton::WriteToStream( BinaryWriter& writer )
{
	//See bottom of .hpp for format.
	bool didWrite = false;

	didWrite = writer.Write<uint32_t>( s_FILE_VERSION );
	didWrite = writer.Write<uint32_t>( m_joints.size() );

	for ( const std::string& name : m_jointNameIDs )
	{
		didWrite = writer.WriteString( name.c_str() );
		if ( !didWrite )
			break;
	}

	for ( int parentJointIndex : m_indicesOfParentJoints )
	{
		didWrite = writer.Write<int>( parentJointIndex );
		if ( !didWrite )
			break;
	}

	//Only writing one of the transforms because we can read it back and invert it to get the other!
	for ( Matrix4x4f& transform : m_globalJointBoneToModelSpaceTransforms )
	{
		didWrite = writer.Write<uint32_t>( transform.GetOrdering() );
		for ( int matrixEntry = 0; matrixEntry < 16; matrixEntry++ )
		{
			didWrite = writer.Write<float>( transform[ matrixEntry ] ); //As in MeshBuilder: per primitive, not per struct, for Endian!
			if ( !didWrite )
				break;
		}
	}

	return didWrite;
}


//--------------------------------------------------------------------------------------------------------------
bool Skeleton::ReadFromFile( const char* filename, int endianMode )
{
	bool didRead = false;

	FileBinaryReader reader;
	reader.SetEndianMode( (EndianMode)endianMode );

	didRead = reader.open( filename );
	if ( !didRead )
	{
		DebuggerPrintf( "File not found in Skeleton::ReadFromFile!" );
		return false;
	}

	didRead = this->ReadFromStream( reader );
	if ( !didRead )
	{
		DebuggerPrintf( "ReadFromStream failed in Skeleton::ReadFromFile!" );
		return false;
	}
	reader.close();

	return didRead;
}

//--------------------------------------------------------------------------------------------------------------
bool Skeleton::ReadOldVersionFormatFromStream( BinaryReader& reader )
{
	UNREFERENCED( reader );
	DebuggerPrintf( "Hit Skeleton::ReadOldVersionFormatFromStream!" );
	return false;
}


//--------------------------------------------------------------------------------------------------------------
bool Skeleton::ReadFromStream( BinaryReader& reader )
{
	//See bottom of .hpp for format.
	bool didRead = false;

	//BE EXPLICIT ABOUT SIZE. size_t differs 32-bit versus 64-bit.
	unsigned int jointCount = 0;

	uint32_t fileVersion;
	didRead = reader.Read<uint32_t>( &fileVersion );

	if ( fileVersion != s_FILE_VERSION )
		return ReadOldVersionFormatFromStream( reader );

	didRead = reader.Read<uint32_t>( &jointCount ); //!\ If Endian mode is wrong, a small count becomes gigantic!
	m_jointNameIDs.resize( jointCount );
	m_indicesOfParentJoints.resize( jointCount );
	m_globalJointBoneToModelSpaceTransforms.resize( jointCount );
	m_unchangingGlobalJointModelToBoneSpaceTransforms.resize( jointCount );

	for ( unsigned int jointIndex = 0; jointIndex < jointCount; jointIndex++ )
	{
		didRead = reader.ReadString( m_jointNameIDs[ jointIndex ] );
		if ( !didRead )
			break;
	}

	for ( unsigned int jointIndex = 0; jointIndex < jointCount; jointIndex++ )
	{
		didRead = reader.Read<int32_t>( &m_indicesOfParentJoints[ jointIndex ] );
		if ( !didRead )
			break;
	}

	ReconstructLocalTransformHierarchy();

	uint32_t matrixOrdering;
	//As in MeshBuilder: per primitive, not per struct, for Endian conversion support!
	for ( unsigned int jointIndex = 0; jointIndex < jointCount; jointIndex++ )
	{
		didRead = reader.Read<uint32_t>( &matrixOrdering );
		Matrix4x4f currentTransform( static_cast<Ordering>( matrixOrdering ) );

		for ( int elementIndex = 0; elementIndex < 16; elementIndex++ )
		{
			didRead = reader.Read<float>( &currentTransform[ elementIndex ] );
			if ( !didRead )
				break;
		}

		m_globalJointBoneToModelSpaceTransforms[ jointIndex ] = currentTransform;
		currentTransform.GetInverseAssumingOrthonormality( currentTransform );
		m_unchangingGlobalJointModelToBoneSpaceTransforms[ jointIndex ] = currentTransform;
	}

	return didRead;
}


//--------------------------------------------------------------------------------------------------------------
void Skeleton::ReconstructLocalTransformHierarchy()
{
	if ( m_indicesOfParentJoints.size() == 0 )
		return;


	m_hierarchyRoot = new SkeletonJoint( 0 );
	m_joints.push_back( m_hierarchyRoot );

	
	for ( unsigned int jointIndex = 1; jointIndex < m_indicesOfParentJoints.size(); jointIndex++ )
	{
		SkeletonJoint* newJoint = new SkeletonJoint( jointIndex );

		//Assumes earlier parent joint indices will always be lower values than latter indices in the array:
		int currentParentJointIndex = m_indicesOfParentJoints[ jointIndex ];
		SkeletonJoint* newJointParent = m_joints[ currentParentJointIndex ];

		newJoint->m_parent = newJointParent;
		newJointParent->m_children.push_back( newJoint );

		m_joints.push_back( newJoint );
	}
}


//--------------------------------------------------------------------------------------------------------------
void Skeleton::RecursivelyDrawBones( SkeletonJoint* currentJoint, const Rgba& boneColor, float boneThickness )
{
	if ( currentJoint->m_children.size() == 0 )
		return;

	Vector3f parentPosition;
	m_globalJointBoneToModelSpaceTransforms[ currentJoint->m_parallelArraysIndex ].GetTranslation( parentPosition );

	for ( SkeletonJoint* childJoint : currentJoint->m_children )
	{
		Vector3f childPosition;
		m_globalJointBoneToModelSpaceTransforms[ childJoint->m_parallelArraysIndex ].GetTranslation( childPosition );

		AddDebugRenderCommand( new DebugRenderCommandLine( parentPosition, childPosition, 0.f, DEPTH_TEST_DUAL, boneColor, boneThickness ) );

		RecursivelyDrawBones( childJoint, boneColor, boneThickness * .1f ); //Down a tenth in width each time.
	}
}


//--------------------------------------------------------------------------------------------------------------
void Skeleton::VisualizeSkeleton( const Rgba& boneColor /*= Rgba::CYAN*/, float boneThickness /*= 1.f*/, const Rgba& jointColor /*= Rgba::MAGENTA*/, float jointSizeScalar /*= .001f*/ )
{
	//Make the root distinct in size and color.
	Vector3f position;
	m_globalJointBoneToModelSpaceTransforms[ ( m_joints[ 0 ] )->m_parallelArraysIndex ].GetTranslation( position );
	AABB3f rootBounds;
	rootBounds.mins = position + Vector3f::ONE * .01f;
	rootBounds.maxs = position - Vector3f::ONE * .01f;
	AddDebugRenderCommand( new DebugRenderCommandAABB3( rootBounds, true, 0.f, DEPTH_TEST_DUAL, 1.f, jointColor, jointColor *  Rgba::GRAY ) );

	for ( std::vector<SkeletonJoint*>::iterator it = m_joints.begin() + 1; it != m_joints.end(); ++it )
	{
		m_globalJointBoneToModelSpaceTransforms[ ( *it )->m_parallelArraysIndex ].GetTranslation( position );
		AddDebugRenderCommand( new DebugRenderCommandPoint( position, 0.f, DEPTH_TEST_DUAL, jointColor, 1.f, jointSizeScalar ) );
	}
	RecursivelyDrawBones( m_hierarchyRoot, boneColor, boneThickness );
}