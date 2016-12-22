#pragma once


#include <map>
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Memory/UntrackedAllocator.hpp"


//--------------------------------------------------------------------------------------------------------------
class FixedBitmapFont;
typedef std::pair< size_t, FixedBitmapFont* > FixedBitmapFontRegistryPair;
typedef std::map< size_t, FixedBitmapFont*, std::less<size_t>, UntrackedAllocator<FixedBitmapFontRegistryPair> > FixedBitmapFontRegistryMap;



//--------------------------------------------------------------------------------------------------------------
class FixedBitmapFont
{
public:
	static FixedBitmapFont* CreateOrGetFont( const std::string& FixedBitmapFontName );
	static const FixedBitmapFontRegistryMap& GetFontRegistry() { return s_fontRegistry; }
	static void ClearFontRegistry() { s_fontRegistry.clear(); }
	AABB2f GetTexCoordsForGlyph( int glyphUnicode ) const;
	Texture* GetFontTexture() const { return m_spriteSheet.GetAtlasTexture(); }

private:
	FixedBitmapFont( const std::string& FixedBitmapFontName );

	static FixedBitmapFontRegistryMap s_fontRegistry;
	SpriteSheet m_spriteSheet;

	static const int BITMAP_FONT_GLYPHS_WIDE = 16;
	static const int BITMAP_FONT_GLYPHS_HIGH = 16;
	static const int BITMAP_FONT_GLYPH_WIDTH = 16;
	static const int BITMAP_FONT_GLYPH_HEIGHT = 16;
};
