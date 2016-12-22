#pragma once


#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Renderer/Rgba.hpp"
#include "Engine/EngineCommon.hpp"


//-----------------------------------------------------------------------------
class SpriteResource;


//-----------------------------------------------------------------------------
enum LoopMode
{
	SPRITE_ANIM_MODE_PLAY_TO_END,	//Play from time=0 to durationSeconds, then finish.
	SPRITE_ANIM_MODE_LOOPING,		//Play from time=0 to end then repeat, never finish.
	SPRITE_ANIM_MODE_PINGPONG,		//Play from time=0 to end, then time=end to 0, repeat.
	NUM_SPRITE_ANIM_MODES
};


//-----------------------------------------------------------------------------
extern LoopMode GetLoopModeForString( const std::string& loopModeStr );


//-----------------------------------------------------------------------------
struct Keyframe
{
	SpriteResource* spriteResource;
	float startTimeSeconds;
};


//-----------------------------------------------------------------------------
class AnimatedSpriteSequence //Also a resource, so it's static, can add as separate SpriteDatabase map.
{
public:
	AnimatedSpriteSequence( const std::string& name, LoopMode loopMode, float durationSeconds )
		: m_name( name )
		, m_loopMode( loopMode )
		, m_durationSeconds( durationSeconds )
	{
	}
	ResourceID GetID() const { return m_name; }
	SpriteResource* GetKeyframe( float timeSeconds ) const;
	float UpdateTime( float inTimeSeconds, float deltaSeconds ) const;  //inTimeSeconds is the time you're updating, since AnimatedSprite has m_time, not AnimationSequence.
	void AddKeyframe( float keyframeStartTimeSeconds, SpriteResource* spriteResource );

private:
	ResourceID m_name;
	std::vector< Keyframe > m_keyframes;
	LoopMode m_loopMode;
	float m_durationSeconds; //How long that last frame lasts. m_endTimeSeconds in notes.
};


//-----------------------------------------------------------------------------
class AnimatedSprite : public Sprite
{
public:
	static AnimatedSprite* Create( ResourceID id, const WorldCoords2D& worldPos, const Rgba& tint = Rgba::WHITE );
	static AnimatedSprite* Create( AnimatedSprite* other );

	ResourceID m_name;
	AnimatedSpriteSequence const* m_sequence; //New kind of SpriteResource, in a sense.
	float m_currentTimeSeconds;
	virtual void Update( float deltaSeconds ) override;
};