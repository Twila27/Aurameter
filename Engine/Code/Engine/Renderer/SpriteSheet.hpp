#pragma once


#include <string>
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/AABB2.hpp"


//-----------------------------------------------------------------------------
class Texture;


//-----------------------------------------------------------------------------
class SpriteSheet
{
public:
	SpriteSheet( const std::string& imageFilePath, int tilesWide, int tilesHigh, int tileWidth, int tileHeight );

	AABB2f GetTexCoordsFromSpriteCoords( int spriteX, int spriteY ) const; // mostly for atlases
	AABB2f GetTexCoordsFromSpriteIndex( int spriteIndex ) const; // mostly for sprite animations, ensure 0-based index
	int GetNumSprites() const;
	Texture* GetAtlasTexture() const { return m_spriteSheetTexture; }


private:
	Texture* 	m_spriteSheetTexture;
	Vector2i	m_spriteLayout;	// # of sprites total (across and down) on the sheet
	Vector2f	m_texelsPerSprite; // One step into a tile, kept as a fraction 1/tileSize.
	Vector2i	m_tileSize;
};
