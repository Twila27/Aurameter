#include "Engine/Renderer/BitmapFont.hpp"


#include "Engine/EngineCommon.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/String/StringUtils.hpp"

#define STBI_HEADER_FILE_ONLY
#include "ThirdParty/stb/stb_image.c"

#define STATIC // Do-nothing indicator that method/member is static in class definition


//---------------------------------------------------------------------------
STATIC BitmapFontRegistryMap BitmapFont::s_fontRegistry;


//--------------------------------------------------------------------------------------------------------------
BitmapFont::BitmapFont( const std::string& fntFilePath )
	: m_fontAtlases( new Texture* )
{
	FILE* file;
	errno_t err = fopen_s( &file, fntFilePath.c_str(), "rb" );
	if ( err != 0 )
		ERROR_AND_DIE( Stringf( "Failed to open %s in BitmapFont constructor.", fntFilePath.c_str() ) );


	//Temps also for handling casting and structuring loops.
	char cstyleFace[80];
	char cstyleCharset[80];
	int pageNum;
	char cstyleImageFilePath[400];
	std::string imageFilePathStr;
	int numKernings;
	int firstKerningPairId;
	int secondKerningPairId;
	int pixelsToMoveBack;
		

	fscanf_s( file,  //%s reads up to space, ' ' eats all whitespace, %[^\"] reads up to next double quote:
			"info face=\"%[^\"]\" size=%d bold=%d italic=%d charset=\"%[^\"]\" unicode=%d stretchH=%d smooth=%d aa=%d padding=%d,%d,%d,%d spacing=%d,%d outline=%d ",
			cstyleFace, _countof(cstyleFace),
			&m_size,
			&m_isBold,
			&m_isItalic,
			cstyleCharset, _countof(cstyleCharset),
			&m_isUnicode,
			&m_stretchHeight,
			&m_isSmoothed,
			&m_usesAntiAliasing,
			&m_paddingTop,
			&m_paddingRight,
			&m_paddingBottom,
			&m_paddingLeft,
			&m_spacingHorizontal,
			&m_spacingVertical,
			&m_outlineThickness
		);

	fscanf( file, 
			"common lineHeight=%d base=%d scaleW=%d scaleH=%d pages=%d packed=%d alphaChnl=%d redChnl=%d greenChnl=%d blueChnl=%d ",
			&m_lineHeight,
			&m_baseHeight,
			&m_textureWidth,
			&m_textureHeight,
			&m_numPages,
			&m_areCharsPackedInMultipleChannels,
			&m_hasAlphaChannel,
			&m_hasRedChannel,
			&m_hasGreenChannel,
			&m_hasBlueChannel
		);

	int imagePathSize = _countof( cstyleImageFilePath );
	for ( int pageIndex = 0; pageIndex < m_numPages; pageIndex++ )
	{
		fscanf_s( file,
			"page id=%d file=\"%[^\"]\" ",
			&pageNum,
			cstyleImageFilePath, imagePathSize
		);
		imageFilePathStr = "Data/Fonts/";
		imageFilePathStr += cstyleImageFilePath;
		LoadFontImage( imageFilePathStr, pageIndex );
	}

	fscanf( file,
		"chars count=%d ",
		&numGlyphs
	);

	m_tallestGlyphPx = 0.f;
	for ( int glyphIndex = 0; glyphIndex < numGlyphs; glyphIndex++ )
	{
		Glyph* newGlyph = new Glyph;
		fscanf( file,
			"char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d page=%d chnl=%d ",
			&newGlyph->m_id,
			&newGlyph->m_x,
			&newGlyph->m_y,
			&newGlyph->m_width,
			&newGlyph->m_height,
			&newGlyph->m_xoffset,
			&newGlyph->m_yoffset,
			&newGlyph->m_xadvance,
			&newGlyph->m_page,
			&newGlyph->m_channel
		);	
		m_fontGlyphs[ newGlyph->m_id ] = newGlyph;

		int glyphHeightWithOffset = newGlyph->m_height + newGlyph->m_yoffset;
		if ( glyphHeightWithOffset > m_tallestGlyphPx )
			m_tallestGlyphPx = static_cast<float>( glyphHeightWithOffset );
	}

	fscanf( file,
		"kernings count=%d ",
		&numKernings
	);

	for ( int kerningPairIndex = 0; kerningPairIndex < numKernings; kerningPairIndex++ )
	{
		fscanf( file,
			"kerning first=%d second=%d amount=%d ",
			&firstKerningPairId,
			&secondKerningPairId,
			&pixelsToMoveBack
		);	
		m_fontGlyphs[ firstKerningPairId ]->m_kernings[ secondKerningPairId ] = pixelsToMoveBack;
	}

	fclose( file );

	m_face = cstyleFace;
	m_charset = cstyleCharset;
}


//--------------------------------------------------------------------------------------------------------------
BitmapFont::~BitmapFont()
{
	for ( int pageIndex = 0; pageIndex < m_numPages; pageIndex++ )
	{
		if ( m_fontAtlases[ pageIndex ] != nullptr )
		{
	//		delete m_fontAtlases[ pageIndex ]; //Cleaned by TheRenderer::DeleteTextures.
			m_fontAtlases[ pageIndex ] = nullptr;
		}
	}

	if ( m_fontAtlases != nullptr )
	{
		delete m_fontAtlases;
		m_fontAtlases = nullptr;
	}

	auto glyphIterEnd = m_fontGlyphs.end();
	for ( auto glyphIter = m_fontGlyphs.begin(); glyphIter != glyphIterEnd; ++glyphIter )
	{
		std::pair< int, Glyph* > glyph = *glyphIter;
		if ( glyph.second != nullptr )
		{
			delete glyph.second;
			glyph.second = nullptr;
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
void BitmapFont::LoadFontImage( const std::string& imageFilePath, unsigned int pageIndex )
{
	//Get the data out of the supplied path.
	Vector2i imageSizeInTexels;
	int numChannels;
	int numChannelsRequested = 0; //Means we don't care.
	byte_t* undupedImageData = stbi_load( imageFilePath.c_str(), &imageSizeInTexels.x, &imageSizeInTexels.y, &numChannels, numChannelsRequested );
	ASSERT_OR_DIE( numChannels == 1, Stringf( "NumChannels != 1 in BitmapFont::LoadFontImage(\"%s\")", imageFilePath.c_str() ) ); //Else below dupe code flops.

	//Manipulate that data; note pixel color components are unsigned bytes, i.e. 1 byte/uchar per channel. Code loop structure taken from MP1/DOT3 project.
	int numTexels = imageSizeInTexels.x * imageSizeInTexels.y;
	int numBytes = numTexels * numChannels;
	byte_t* dupedImageData = new byte_t[ numBytes * 4 ]; //Duplicate font image's 1-channel "R" data across 4-channels "Rgba".
	for ( int i = 0; i < numBytes; i += numChannels ) //Loop over all bytes, not just # texels.
	{
		dupedImageData[ 4*i + 0 ] = undupedImageData[ i ]; //"R" channel.
		dupedImageData[ 4*i + 1 ] = undupedImageData[ i ]; //"G" channel.
		dupedImageData[ 4*i + 2 ] = undupedImageData[ i ]; //"B" channel.
		dupedImageData[ 4*i + 3 ] = undupedImageData[ i ]; //"A" channel.
	}
	
	stbi_image_free( undupedImageData );

	m_fontAtlases[ pageIndex ] = Texture::CreateTextureFromBytes( imageFilePath, dupedImageData, imageSizeInTexels, 4 );

	delete[] dupedImageData;
}


//--------------------------------------------------------------------------------------------------------------
const Glyph* BitmapFont::GetGlyphForChar( char c ) const
{
	if ( m_fontGlyphs.count( c ) > 0 )
		return m_fontGlyphs.at( c );
	else 
		return nullptr;
}


//--------------------------------------------------------------------------------------------------------------
BitmapFont* BitmapFont::CreateOrGetFont( const std::string& fntFilePath )
{
	size_t pathHash = std::hash<std::string>{}( fntFilePath );
	if ( s_fontRegistry.find( pathHash ) != s_fontRegistry.end( ) ) return s_fontRegistry[ pathHash ];
	else s_fontRegistry[ pathHash ] = new BitmapFont( fntFilePath );
	
	return s_fontRegistry[ pathHash ];
}


//--------------------------------------------------------------------------------------------------------------
Texture* BitmapFont::GetFontTexture( int pageNum ) const
{
	GUARANTEE_OR_DIE( pageNum < m_numPages, Stringf( "Array m_fontAtlases exceeded in BitmapFont::GetFontTexture(%d).", pageNum ) );
	return m_fontAtlases[ pageNum ];
}


//--------------------------------------------------------------------------------------------------------------
TODO( "Switch / for * as in SpriteSheet's approach." );
AABB2f BitmapFont::GetTexCoordsForGlyph( int glyphUnicode ) const
{
	//Trying to return a mins and a maxs relative to the texture itself.
	ASSERT_OR_DIE( m_fontGlyphs.count( glyphUnicode ) > 0, Stringf("BitmapFont::GetTexCoordsForGlyph() failed to find %d in m_fontGlyphs!", glyphUnicode) );
	Glyph* glyph = m_fontGlyphs.at( glyphUnicode );

	//Normalize via dividing full image width and height, because the full texCoords == [0,1]x[0,1].
	float topLeftX = glyph->m_x / static_cast<float>( m_textureWidth );
	float topLeftY = glyph->m_y / static_cast<float>( m_textureHeight );
	float bottomRightX = ( glyph->m_x + glyph->m_width ) / static_cast<float>( m_textureWidth );
	float bottomRightY = ( glyph->m_y + glyph->m_height ) / static_cast<float>( m_textureWidth );

	return AABB2f( topLeftX, topLeftY, bottomRightX, bottomRightY );
}
