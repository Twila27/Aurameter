#pragma once


#include "Engine/Math/Matrix4x4.hpp"


//-----------------------------------------------------------------------------
class Skeleton;
class AnimationSequence;
extern AnimationSequence* g_lastLoadedAnimation;
class BinaryReader;
class BinaryWriter;


//-----------------------------------------------------------------------------
enum AnimationMode
{
	ANIMATION_MODE_CLAMP,
	ANIMATION_MODE_LOOP,
	ANIMATION_MODE_PINGPONG,
	NUM_ANIMATION_MODES
};


//-----------------------------------------------------------------------------
class AnimationSequence //Motion in class notes. Recommended to call it KeyframeMotion to distinguish from AnimationCurveMotion...?
{
	public:
		AnimationSequence() {} //Mostly for loading purposes.
		AnimationSequence( const std::string& name, float durationSeconds, float framerate, Skeleton* initialTarget, uint32_t useLocalOverGlobalTransform );
		~AnimationSequence() { delete[] m_keyframes; }


		//Will have an initial internal skeleton, and if you don't want to support retargeting, it needs only needs to know that initial skeleton.

		uint32_t m_useLocalOverGlobalTransform;
		std::string m_name;
		uint32_t m_numKeyframesPerJoint;
		float m_animationLengthSeconds;
		float m_keyframeRate; //FPS.
		float m_keyframeLengthSeconds; //Amount of time between each keyframe. SPF.
		uint32_t m_numJoints;

		// 2D array of matrices, stride is sizeof(Matrix4x4) * numJoints, i.e. this is the size of a single joint keyframe.
		Matrix4x4f* m_keyframes; //secretly [numJoints][numKeyframesPerJoint]; but would need to know it at compile-time. Still stored 1D array though.

		void GetFrameIndicesWithBlend( uint32_t& out_startingKeyframeIndex, uint32_t& out_endingKeyframeIndex, float& out_blend, float in_normalizedTime ); //Normalized from 0 to m_animLength.
			//Blend is %interpolate between just the two keyframes, since t is across the entire animation.

		Matrix4x4f* GetKeyframesForJoint( uint32_t jointIndex )
		{
			return m_keyframes + ( jointIndex * m_numKeyframesPerJoint );
		}

		//Walk each skeleton joint, get 2 closest keyframes for those joints, lerp between them, and set it on the skeleton. This is all the "application" is.
		void ApplyMotionToSkeleton( Skeleton* skeleton, float in_normalizedTime ); //the instantaneous "t" to evaluate at, so no units? //Normalized from 0 to m_animLength.

		bool WriteToFile( const char* filename, bool appendToFile, int endianMode );
		bool WriteToStream( BinaryWriter& writer );
		bool ReadFromFile( const char* filename, int endianMode );
		bool ReadFromStream( BinaryReader& reader );


private:
		bool ReadOldVersionFormatFromStream( BinaryReader& reader );
		static const uint32_t s_FILE_VERSION = 1;
};



/* AnimationSequence Serialization Format v1.0 (AES A05)
	1. FILE VERSION -- this way we don't try to load an old format, just forbid it.
	2. String Name
	3. uint32_t m_useLocalOverGlobalTransform
	4. float m_animationLengthSeconds
	5. float m_keyframeRate
	6. uint32_t m_numJoints
	7. Matrix4x4 m_keyframes[#KeyframesPerJoint * #Joints]
*/
