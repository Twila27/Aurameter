#include "Engine/Renderer/SpriteSheet.hpp"


#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/AABB2.hpp"


//--------------------------------------------------------------------------------------------------------------
SpriteSheet::SpriteSheet( const std::string& imageFilePath, int tilesWide, int tilesHigh, int tileWidth, int tileHeight )
	: m_spriteSheetTexture( Texture::CreateOrGetTexture( imageFilePath ) )
	, m_spriteLayout( tilesWide, tilesHigh )
	, m_tileSize( tileWidth, tileHeight )
	, m_texelsPerSprite( 1.f / (float) tilesWide, 1.f / (float) tilesHigh )
{
}


//--------------------------------------------------------------------------------------------------------------
AABB2f SpriteSheet::GetTexCoordsFromSpriteCoords( int spriteX, int spriteY ) const
{
	AABB2f texCoords = AABB2f(0.f, 0.f, 0.f, 0.f);
	texCoords.mins.x = m_texelsPerSprite.x * static_cast<float>( spriteX );
	texCoords.mins.y = m_texelsPerSprite.y * static_cast<float>( spriteY );
	texCoords.maxs = texCoords.mins + Vector2f( (float) m_texelsPerSprite.x , (float) m_texelsPerSprite.y );

	return texCoords;
}


//--------------------------------------------------------------------------------------------------------------
AABB2f SpriteSheet::GetTexCoordsFromSpriteIndex( int spriteIndex ) const
{
	//Important to be sure the index is 0-based, but be sure the # rows/cols are not.
	int spriteX = spriteIndex % m_spriteLayout.x; 
	int spriteY = spriteIndex / m_spriteLayout.x;
	//Note spriteX, spriteY are the top-left of the sprite.

	return GetTexCoordsFromSpriteCoords( spriteX, spriteY );
}


//--------------------------------------------------------------------------------------------------------------
int SpriteSheet::GetNumSprites() const
{
	return m_spriteLayout.x * m_spriteLayout.y;
}