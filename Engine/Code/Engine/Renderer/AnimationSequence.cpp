#include "Engine/Renderer/AnimationSequence.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Skeleton.hpp"
#include "Engine/FileUtils/Writers/FileBinaryWriter.hpp"
#include "Engine/FileUtils/Readers/FileBinaryReader.hpp"


//--------------------------------------------------------------------------------------------------------------
AnimationSequence* g_lastLoadedAnimation = nullptr;


//--------------------------------------------------------------------------------------------------------------
AnimationSequence::AnimationSequence( const std::string& name, float durationSeconds, float framerate, Skeleton* initialTarget, uint32_t useLocalOverGlobalTransform )
	: m_name( name )
	, m_useLocalOverGlobalTransform( useLocalOverGlobalTransform )
	, m_numKeyframesPerJoint( static_cast<uint32_t>( ceil( framerate * durationSeconds ) + 1 ) )
	, m_numJoints( initialTarget->GetNumJoints() )
	, m_animationLengthSeconds( durationSeconds )
	, m_keyframeLengthSeconds( 1.f / framerate )
	, m_keyframeRate( framerate )
	, m_keyframes( new Matrix4x4f[ m_numKeyframesPerJoint * m_numJoints ] )
{
	for ( unsigned int i = 0; i < m_numJoints * m_numKeyframesPerJoint; i++ )
		m_keyframes[ i ].SetToTranspose( true );
}


//--------------------------------------------------------------------------------------------------------------
void AnimationSequence::GetFrameIndicesWithBlend( uint32_t& out_startingKeyframeIndex, uint32_t& out_endingKeyframeIndex, float& out_blend, float in_normalizedTime )
{
	uint32_t startingKeyframeIndex = static_cast<uint32_t>( floor( in_normalizedTime / m_keyframeLengthSeconds ) );
	uint32_t endingKeyframeIndex = startingKeyframeIndex + 1;

	if ( startingKeyframeIndex == ( m_numKeyframesPerJoint - 1 ) ) //Special case 1: keyframeIndex1 just off the end of our key-frame array!
	{
		endingKeyframeIndex = m_numKeyframesPerJoint - 1; //i.e. last frame.
		out_blend = 0.0f;
	}
	else if ( startingKeyframeIndex == ( m_numKeyframesPerJoint - 2 ) ) //Special case 2: keyframeIndex1 at the end of our key-frame array!
	{
		float lastKeyframeLengthSeconds = m_animationLengthSeconds - ( m_keyframeRate * startingKeyframeIndex );
		out_blend = fmodf( in_normalizedTime, m_keyframeLengthSeconds ) / lastKeyframeLengthSeconds;
		out_blend = Clamp( out_blend, 0.f, 1.f );
	}
	else
	{
		out_blend = fmodf( in_normalizedTime, m_keyframeLengthSeconds ) / m_keyframeLengthSeconds;
	}

	out_startingKeyframeIndex = startingKeyframeIndex;
	out_endingKeyframeIndex = endingKeyframeIndex;
}


//--------------------------------------------------------------------------------------------------------------
void AnimationSequence::ApplyMotionToSkeleton( Skeleton* skeleton, float in_normalizedTime ) //Normalized from 0 to m_animLength.
{
	uint32_t startingKeyframe;
	uint32_t endingKeyframe;
	float currentBlendTime;
	GetFrameIndicesWithBlend( startingKeyframe, endingKeyframe, currentBlendTime, in_normalizedTime );

	for ( uint32_t jointIndex = 0; jointIndex < m_numJoints; jointIndex++ )
	{
		Matrix4x4f* jointKeyframes = GetKeyframesForJoint( jointIndex );
		Matrix4x4f& startingTransform = jointKeyframes[ startingKeyframe ];
		Matrix4x4f& endingTransform = jointKeyframes[ endingKeyframe ];

		//Blends 2 matrices. Slerp is cleanest--best results, but takes most time:
		Matrix4x4f transformAtCurrentBlendTime = MatrixLerp( startingTransform, endingTransform, currentBlendTime );

		if ( m_useLocalOverGlobalTransform == 0 )
			skeleton->SetGlobalModelSpaceTransformForJointIndex( jointIndex, transformAtCurrentBlendTime ); //Initial model/world-to-bone NEVER ever changes, we change bone-to-model/world.
		else
			skeleton->SetLocalJointSpaceTransformForJointIndex( jointIndex, transformAtCurrentBlendTime );
		//If only storing a change/delta at that node instead of a Matrix4x4, you would call an ApplyMotionToJoint().
	}
}


//--------------------------------------------------------------------------------------------------------------
bool AnimationSequence::WriteToFile( const char* filename, bool appendToFile, int endianMode )
{
	bool didWrite = false;

	FileBinaryWriter writer;
	writer.SetEndianMode( (EndianMode)endianMode );

	didWrite = writer.open( filename, appendToFile );
	if ( !didWrite )
	{
		DebuggerPrintf( "File not accessible in AnimationSequence::WriteToFile!" );
		return false;
	}

	didWrite = this->WriteToStream( writer );
	if ( !didWrite )
	{
		DebuggerPrintf( "WriteToStream failed in AnimationSequence::WriteToFile!" );
		return false;
	}
	writer.close();

	return didWrite;
}


//--------------------------------------------------------------------------------------------------------------
bool AnimationSequence::WriteToStream( BinaryWriter& writer )
{
	//See bottom of .hpp for format.
	bool didWrite = false;

	didWrite = writer.Write<uint32_t>( s_FILE_VERSION );
	didWrite = writer.WriteString( m_name.c_str() );
	didWrite = writer.Write<uint32_t>( m_useLocalOverGlobalTransform );
	didWrite = writer.Write<float>( m_animationLengthSeconds );
	didWrite = writer.Write<float>( m_keyframeRate );
	didWrite = writer.Write<uint32_t>( m_numJoints );

	for ( uint32_t jointIndex = 0; jointIndex < m_numJoints; jointIndex++ )
	{
		Matrix4x4f* startingKeyframeForJoint = GetKeyframesForJoint( jointIndex );
		for ( uint32_t keyframeIndex = 0; keyframeIndex < m_numKeyframesPerJoint; keyframeIndex++ )
		{
			Matrix4x4f& transform = *( startingKeyframeForJoint + keyframeIndex ); //Pointer arithmetic.
			didWrite = writer.Write<uint32_t>( transform.GetOrdering() );
			for ( int matrixEntry = 0; matrixEntry < 16; matrixEntry++ )
			{
				didWrite = writer.Write<float>( transform[ matrixEntry ] ); //As in MeshBuilder: per primitive, not per struct, for Endian!
				if ( !didWrite )
					break;
			}
		}
	}

	return didWrite;
}


//--------------------------------------------------------------------------------------------------------------
bool AnimationSequence::ReadFromFile( const char* filename, int endianMode )
{
	bool didRead = false;

	FileBinaryReader reader;
	reader.SetEndianMode( (EndianMode)endianMode );

	didRead = reader.open( filename );
	if ( !didRead )
	{
		DebuggerPrintf( "File not found in AnimationSequence::ReadFromFile!" );
		return false;
	}

	didRead = this->ReadFromStream( reader );
	if ( !didRead )
	{
		DebuggerPrintf( "ReadFromStream failed in AnimationSequence::ReadFromFile!" );
		return false;
	}
	reader.close();

	return didRead;
}

//--------------------------------------------------------------------------------------------------------------
bool AnimationSequence::ReadOldVersionFormatFromStream( BinaryReader& reader )
{
	UNREFERENCED( reader );
	DebuggerPrintf( "Hit AnimationSequence::ReadOldVersionFormatFromStream!" );
	return false;
}


//--------------------------------------------------------------------------------------------------------------
bool AnimationSequence::ReadFromStream( BinaryReader& reader )
{
	//See bottom of .hpp for format.
	bool didRead = false;

	uint32_t fileVersion;
	didRead = reader.Read<uint32_t>( &fileVersion );
	if ( fileVersion != s_FILE_VERSION )
		return ReadOldVersionFormatFromStream( reader );

	didRead = reader.ReadString( m_name );
	didRead = reader.Read<uint32_t>( &m_useLocalOverGlobalTransform );
	didRead = reader.Read<float>( &m_animationLengthSeconds );
	didRead = reader.Read<float>( &m_keyframeRate );
	didRead = reader.Read<uint32_t>( &m_numJoints );

	ASSERT_OR_DIE( m_keyframeRate != 0.f, nullptr );
	m_keyframeLengthSeconds = 1.f / m_keyframeRate;

	m_numKeyframesPerJoint = static_cast<uint32_t>( ceil( m_keyframeRate * m_animationLengthSeconds ) + 1 );

	m_keyframes = new Matrix4x4f[ m_numKeyframesPerJoint * m_numJoints ];

	uint32_t matrixOrdering;
	//As in MeshBuilder: per primitive, not per struct, for Endian conversion support!
	for ( unsigned int jointIndex = 0; jointIndex < m_numJoints; jointIndex++ )
	{
		Matrix4x4f* startingKeyframeForJoint = GetKeyframesForJoint( jointIndex );
		for ( uint32_t keyframeIndex = 0; keyframeIndex < m_numKeyframesPerJoint; keyframeIndex++ )
		{
			didRead = reader.Read<uint32_t>( &matrixOrdering );
			Matrix4x4f currentKeyframeTransform( static_cast<Ordering>( matrixOrdering ) );

			for ( int elementIndex = 0; elementIndex < 16; elementIndex++ )
			{
				didRead = reader.Read<float>( &currentKeyframeTransform[ elementIndex ] );
				if ( !didRead )
					break;
			}


			Matrix4x4f* jointKeyframe = startingKeyframeForJoint + keyframeIndex; //Pointer arithmetic.
			*jointKeyframe = currentKeyframeTransform;
		}
	}

	return didRead;
}