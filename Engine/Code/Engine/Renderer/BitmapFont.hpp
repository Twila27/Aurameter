#pragma once


#include <map>
#include "Engine/Math/AABB2.hpp"
#include "Engine/Memory/UntrackedAllocator.hpp"


//-----------------------------------------------------------------------------
class Texture;


//-----------------------------------------------------------------------------
struct Glyph
{
	int m_id;
	int m_x; //These positions refer to those for the character image in the texture.
	int m_y;
	int m_width; //In texels (and px, contingent on use), not normalized texCoords.
	int m_height;
	int m_xoffset; //"How much the current position should be offset when copying the image from the texture to the screen."
	int m_yoffset;
	int m_xadvance; //"How much the current position should be advanced after drawing the character." (Prior to any kerning pair effects.)
	int m_page; //"The texture page where the character image is found."
	int m_channel; //"The texture channel where the character image is found (1 = blue, 2 = green, 4 = red, 8 = alpha, 15 = all channels)."
	std::map< int, int > m_kernings; //If this glyph precedes one of these glyph IDs, offset backward by the associated amount, else return 0.
	float GetKerning( int glyphID ) const { return ( m_kernings.count( glyphID ) > 0 ) ? static_cast<float>( m_kernings.at( glyphID ) ) : 0.f;  }
};


//-----------------------------------------------------------------------------
class BitmapFont;
typedef std::pair< size_t, BitmapFont* > BitmapFontRegistryPair;
typedef std::map< size_t, BitmapFont*, std::less<size_t>, UntrackedAllocator<BitmapFontRegistryPair> > BitmapFontRegistryMap;


//-----------------------------------------------------------------------------
class BitmapFont
{
public:

	static BitmapFont* CreateOrGetFont( const std::string& fntFilePath );
	static const BitmapFontRegistryMap& GetFontRegistry() { return s_fontRegistry; }
	static void ClearFontRegistry() { s_fontRegistry.clear(); }
	~BitmapFont();

	float GetTallestGlyphHeightPx() const { return m_tallestGlyphPx; }
	const Glyph* GetGlyphForChar( char c ) const;
	AABB2f GetTexCoordsForGlyph( int glyphUnicode ) const; //Can't rely on SpriteSheet now, need to call corresponding Glyph object.
	Texture* GetFontTexture( int pageNum = 0 ) const;

private:

	BitmapFont( const std::string& fntFilePath );

	//For file format jargon: http://www.angelcode.com/products/bmfont/doc/file_format.html.

	//From 'info' section of .fnt file.
	std::string m_face; //e.g. Arial, InputSans.
	float m_tallestGlyphPx; //e.g. to size draw calls around.
	int m_size; //The size of the TrueType font the bitmap was taken from.
	int m_isBold;
	int m_isItalic;
	std::string m_charset;
	int m_isUnicode;
	int m_stretchHeight;
	int m_isSmoothed;
	int m_usesAntiAliasing;
	int m_paddingTop;
	int m_paddingRight;
	int m_paddingBottom;
	int m_paddingLeft;
	int m_spacingHorizontal;
	int m_spacingVertical;
	int m_outlineThickness;

	//From 'common' section of .fnt file.
	int m_lineHeight;
	int m_baseHeight; //Used to offset characters like underlining, '.' and ',' whose tops aren't quite to the floor.
	int m_textureWidth;
	int m_textureHeight;
	int m_areCharsPackedInMultipleChannels;
	int m_hasAlphaChannel;
	int m_hasRedChannel;
	int m_hasGreenChannel;
	int m_hasBlueChannel;

	int numGlyphs; //"numChars" in .fnt.

	static BitmapFontRegistryMap s_fontRegistry;	//Uses .fnt filepath as a key--only ever one .fnt file even for multi-page bitmap fonts.
	std::map< int, Glyph* > m_fontGlyphs; //The .fnt char id is the key.

	int m_numPages;
	Texture** m_fontAtlases; //Array of Texture* for multi-page fonts.

	void LoadFontImage( const std::string& imageFilePath, unsigned int pageIndex );
};
