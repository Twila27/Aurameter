#include "Engine/Renderer/SpriteAnimation.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"


//--------------------------------------------------------------------------------------------------------------
SpriteAnimation::SpriteAnimation( const SpriteSheet& spriteSheet, float durationSeconds, SpriteAnimMode mode, int startSpriteIndex, int numSpritesInAnimation )
	: m_spriteSheet( &spriteSheet )
	, m_spriteAnimationMode( mode )
	, m_animationDirection( FORWARD )
	, m_durationSeconds( durationSeconds )
	, m_elapsedSeconds( 0.f )
	, m_isFinished( false )
	, m_isPlaying( true )
	, m_startSpriteIndex( startSpriteIndex )
	, m_numSpritesInAnimation ( numSpritesInAnimation )
{
}


//--------------------------------------------------------------------------------------------------------------
void SpriteAnimation::Update( float deltaSeconds )
{ 
	if ( m_isPlaying ) 
		m_elapsedSeconds += deltaSeconds * m_animationDirection; 
	
	if ( m_elapsedSeconds > m_durationSeconds )
	{
		switch ( m_spriteAnimationMode )
		{
		case SPRITE_ANIM_MODE_PLAY_TO_END:
			m_isFinished = true;
			m_isPlaying = false;
			break;
		case SPRITE_ANIM_MODE_LOOPING:
			m_elapsedSeconds = 0.f;
			break;
		case SPRITE_ANIM_MODE_PINGPONG:
			m_animationDirection = BACKWARD;
			break;
		}
	}
	else if ( m_elapsedSeconds < 0.f ) m_animationDirection = FORWARD;
}


//--------------------------------------------------------------------------------------------------------------
AABB2f SpriteAnimation::GetCurrentTexCoords() const
{
	ASSERT_OR_DIE( m_durationSeconds != 0.f, "SpriteAnimation::m_durationSeconds Cannot Be Zero!" );
	
	float animationCompletionPercentage = m_elapsedSeconds / m_durationSeconds;
	if ( animationCompletionPercentage > 1.f ) animationCompletionPercentage = 1.f;
	
	int spriteIndex = m_startSpriteIndex;
	int spriteIndexTotal = ( m_numSpritesInAnimation - 1 ) - m_startSpriteIndex;
	spriteIndex += (int) floor( animationCompletionPercentage * spriteIndexTotal );

	return m_spriteSheet->GetTexCoordsFromSpriteIndex( spriteIndex );
}


//--------------------------------------------------------------------------------------------------------------
Texture* SpriteAnimation::GetTexture() const
{ 
	return m_spriteSheet->GetAtlasTexture( ); 
}