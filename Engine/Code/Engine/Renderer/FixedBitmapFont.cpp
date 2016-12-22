#include "Engine/Renderer/FixedBitmapFont.hpp"


#include "Engine/EngineCommon.hpp"

#define STATIC // Do-nothing indicator that method/member is static in class definition


//---------------------------------------------------------------------------
STATIC FixedBitmapFontRegistryMap FixedBitmapFont::s_fontRegistry;


//--------------------------------------------------------------------------------------------------------------
FixedBitmapFont::FixedBitmapFont( const std::string& fixedBitmapFontName )
	: m_spriteSheet( fixedBitmapFontName, BITMAP_FONT_GLYPHS_WIDE, BITMAP_FONT_GLYPHS_HIGH, BITMAP_FONT_GLYPH_WIDTH, BITMAP_FONT_GLYPH_HEIGHT )
{
}


//--------------------------------------------------------------------------------------------------------------
FixedBitmapFont* FixedBitmapFont::CreateOrGetFont( const std::string& fixedBitmapFontName )
{
	size_t pathHash = std::hash<std::string>{}( fixedBitmapFontName );
	if ( s_fontRegistry.find( pathHash ) != s_fontRegistry.end() ) return s_fontRegistry[ pathHash ];
	else s_fontRegistry[ pathHash ] = new FixedBitmapFont( fixedBitmapFontName );

	return s_fontRegistry[ pathHash ];
}


//--------------------------------------------------------------------------------------------------------------
AABB2f FixedBitmapFont::GetTexCoordsForGlyph( int glyphUnicode ) const
{
	return m_spriteSheet.GetTexCoordsFromSpriteIndex( glyphUnicode ); //Assumes ASCII maps directly to indices.
}