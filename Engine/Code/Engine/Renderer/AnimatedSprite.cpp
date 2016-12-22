#include "Engine/Renderer/AnimatedSprite.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/String/StringUtils.hpp"
#include "Engine/Renderer/ResourceDatabase.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC AnimatedSprite* AnimatedSprite::Create( ResourceID id, const WorldCoords2D& worldPos, const Rgba& tint /*= Rgba::WHITE */ )
{
	AnimatedSprite* anim = new AnimatedSprite();
	anim->m_sequence = ResourceDatabase::Instance()->GetSpriteSequence( id );
	anim->m_name = id;

	// If you enable it by default and are doing multi-threading, 
	// you'll get frames where the renderer's rendering it halfway through its initialization 
	// (if there are other init steps handled by other threads).
	anim->m_spriteID = s_BASE_SPRITE_ID++;
	anim->m_layerID = 0;
	anim->m_overrideMaterial = nullptr;

	anim->m_spriteResource = anim->m_sequence->GetKeyframe( 0.f );

	anim->m_transform.m_position = worldPos;
	anim->m_transform.m_scale = 1.f;
	anim->m_transform.m_rotationAngleDegrees = 0.f;
	anim->m_tint = tint;

	return anim;
}


//--------------------------------------------------------------------------------------------------------------
STATIC AnimatedSprite* AnimatedSprite::Create( AnimatedSprite* other )
{
	if ( other == nullptr )
		return nullptr;

	return AnimatedSprite::Create( other->m_name, other->m_transform.m_position, other->GetTint() );
}


//--------------------------------------------------------------------------------------------------------------
void AnimatedSprite::Update( float deltaSeconds )
{
	m_currentTimeSeconds = m_sequence->UpdateTime( m_currentTimeSeconds, deltaSeconds );
	m_spriteResource = m_sequence->GetKeyframe( m_currentTimeSeconds ); //Updating it in-place, so that next time the SpriteRenderer comes along, it uses the new sprite.
}


//--------------------------------------------------------------------------------------------------------------
SpriteResource* AnimatedSpriteSequence::GetKeyframe( float timeSeconds ) const
{
	ASSERT_OR_DIE( m_keyframes.size() > 0, "No Keyframes to get in AnimatedSpriteSequence::GetKeyframe!" );

	float clampedTime = timeSeconds;
	switch ( m_loopMode )
	{
		case SPRITE_ANIM_MODE_PLAY_TO_END: 
		{
			if ( timeSeconds > m_durationSeconds )
				clampedTime = m_durationSeconds;
			break;
		}
		case SPRITE_ANIM_MODE_PINGPONG:
		{
			int quotient = (int)( timeSeconds / m_durationSeconds );

			if ( quotient & 1 ) //Is quotient odd? Then we're going backwards--every animation runs forward an even-th time: 0th time, 2nd, 4th...
				clampedTime = m_durationSeconds - WrapNumberWithinCircularRange( timeSeconds, 0.f, m_durationSeconds );
			else
				clampedTime = WrapNumberWithinCircularRange( timeSeconds, 0.f, m_durationSeconds );
			break;
		}
		case SPRITE_ANIM_MODE_LOOPING: 
		{
			clampedTime = WrapNumberWithinCircularRange( timeSeconds, 0.f, m_durationSeconds );
			break;
		}
		default: ERROR_RECOVERABLE( "Unexpected loop mode in AnimatedSpriteSequence::GetKeyframe!" ); break;
	}

	for ( const Keyframe& frame : m_keyframes )
		if ( frame.startTimeSeconds >= clampedTime )
			return frame.spriteResource;

	return m_keyframes.back().spriteResource;
}


//--------------------------------------------------------------------------------------------------------------
float AnimatedSpriteSequence::UpdateTime( float inTimeSeconds, float deltaSeconds ) const
{
	return inTimeSeconds + deltaSeconds;

	//Double times inTime, and do the clamping in GetTime.
	//If it's a loop or clamp mode, it just returns the final time for this sequence.
	//Could be sent by-reference, but we're just returning the result.
}


//--------------------------------------------------------------------------------------------------------------
void AnimatedSpriteSequence::AddKeyframe( float keyframeStartTimeSeconds, SpriteResource* spriteResource )
{
	Keyframe frame;
	frame.startTimeSeconds = keyframeStartTimeSeconds;
	frame.spriteResource = spriteResource;
	m_keyframes.push_back( frame );
}


//--------------------------------------------------------------------------------------------------------------
extern LoopMode GetLoopModeForString( const std::string& loopModeStr )
{
	std::string lowered = GetAsLowercase( loopModeStr );
	if ( lowered == "looping" )
		return SPRITE_ANIM_MODE_LOOPING;
	if ( lowered == "pingpong" )
		return SPRITE_ANIM_MODE_PINGPONG;
	else
		return SPRITE_ANIM_MODE_PLAY_TO_END;
}
