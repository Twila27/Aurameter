#pragma once


#include "Engine/Math/AABB2.hpp"


//-----------------------------------------------------------------------------
class SpriteSheet;
class Texture;


//-----------------------------------------------------------------------------
enum SpriteAnimMode
{
	SPRITE_ANIM_MODE_PLAY_TO_END,	//Play from time=0 to durationSeconds, then finish.
	SPRITE_ANIM_MODE_LOOPING,		//Play from time=0 to end then repeat, never finish.
	SPRITE_ANIM_MODE_PINGPONG,		//Play from time=0 to end, then time=end to 0, repeat.
	NUM_SPRITE_ANIM_MODES
};

enum SpriteAnimDir
{
	FORWARD = 1,
	BACKWARD = -1
};

class SpriteAnimation
{
public:
	SpriteAnimation( const SpriteSheet& spriteSheet, float durationSeconds,
		SpriteAnimMode mode, int startSpriteIndex, int numSpritesInAnimation );
	//No dtor until we've learned how to handle deleting that static texture registery.

	void Update( float deltaSeconds );
	AABB2f GetCurrentTexCoords() const; //Based on current elapsed time.
	Texture* GetTexture() const;
	void Pause()						{ m_isPlaying = false; } //Starts unpaused (playing) by default.
	void Resume()						{ m_isPlaying = true; } //Resume as in unpause after playing has started.
	void Reset()						{ m_elapsedSeconds = 0.f; m_isFinished = false; m_isPlaying = true; } //Rewind time=0 and starts [re]playing.
	bool IsFinished() const				{ return m_isFinished; }
	bool IsPlaying() const				{ return m_isPlaying; }
	float GetDurationSeconds() const	{ return m_durationSeconds; }
	float GetSecondsElapsed() const		{ return m_elapsedSeconds; }
	float GetSecondsRemaining() const	{ return m_durationSeconds - m_elapsedSeconds; }
	void SetSecondsElapsed( float secondsElapsed ) { m_elapsedSeconds = secondsElapsed; } //Jump to specific time.
	void SetFractionElapsed( float fractionElapsed ) { m_elapsedSeconds = fractionElapsed * m_durationSeconds; } //e.g. 1.f/3.f for one-third in.

private:
	const SpriteSheet* m_spriteSheet;
	SpriteAnimMode m_spriteAnimationMode;
	SpriteAnimDir m_animationDirection;
	bool m_isFinished;
	bool m_isPlaying;
	float m_durationSeconds;
	float m_elapsedSeconds;
	int m_startSpriteIndex;
	int m_numSpritesInAnimation; //Makes it clear that it's 1-based, where endSpriteIndex doesn't to an end user.
};