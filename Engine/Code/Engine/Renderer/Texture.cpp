//---------------------------------------------------------------------------
// Based on code written by Squirrel Eiserloh
//
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/Renderer/OpenGLExtensions.hpp"


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#include "glext.h"

#define STBI_HEADER_FILE_ONLY
#include "ThirdParty/stb/stb_image.c"

#define STATIC // Do-nothing indicator that method/member is static in class definition


//---------------------------------------------------------------------------
STATIC TextureRegistryMap Texture::s_textureRegistry;


//---------------------------------------------------------------------------
Texture::Texture( const std::string& imageFilePath )
	: m_openglTextureID( 0 )
	, m_sizeInTexels( 0, 0 )
	, m_filepath( imageFilePath )
{
	int numChannels = 0; // Filled in for us to indicate how many color/alpha components the image had (e.g. 3=RGB, 4=Rgba)
	int numChannelsRequested = 0; // don't care; we support 3 (RGB) or 4 (Rgba)
	unsigned char* imageData = stbi_load( imageFilePath.c_str(), &m_sizeInTexels.x, &m_sizeInTexels.y, &numChannels, numChannelsRequested );

	// Enable texturing
	glEnable( GL_TEXTURE_2D );

	// Tell OpenGL that our pixel data is single-byte aligned
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	// Ask OpenGL for an unused texName (ID number) to use for this texture
	glGenTextures( 1, (GLuint*)&m_openglTextureID );

	// Tell OpenGL to bind (set) this as the currently active texture
	glBindTexture( GL_TEXTURE_2D, m_openglTextureID );

	// Set texture clamp vs. wrap (repeat)
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP ); // one of: GL_CLAMP or GL_REPEAT
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP ); // one of: GL_CLAMP or GL_REPEAT

	// Set magnification (texel > pixel) and minification (texel < pixel) filters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );  // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR

	GLenum bufferFormat = GL_RGBA; // the format our source pixel data is currently in; any of: GL_RGB, GL_Rgba, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
	if ( numChannels == 3 )
		bufferFormat = GL_RGB;

	// #FIXME: What happens if numComponents is neither 3 nor 4?

	GLenum internalFormat = bufferFormat; // the format we want the texture to me on the card; allows us to translate into a different texture format as we upload to OpenGL

	glTexImage2D(			// Upload this pixel data to our new OpenGL texture
		GL_TEXTURE_2D,		// Creating this as a 2d texture
		0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
		internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
		m_sizeInTexels.x,		// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,10], and B is the border thickness [0,1]
		m_sizeInTexels.y,		// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,10], and B is the border thickness [0,1]
		0,					// Border size, in texels (must be 0 or 1)
		bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
		GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color/alpha channel)
		imageData );		// Location of the actual pixel data bytes/buffer

	stbi_image_free( imageData );

	glDisable( GL_TEXTURE_2D );
}


//---------------------------------------------------------------------------
Texture::Texture( const std::string& textureName, const unsigned char* imageData, const Vector2i& textureSize, unsigned int numComponents )
	: m_openglTextureID( 0 )
	, m_sizeInTexels( textureSize )
	, m_filepath( textureName )
{	
	// Enable texturing
	glEnable( GL_TEXTURE_2D );

	// Tell OpenGL that our pixel data is single-byte aligned
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	// Ask OpenGL for an unused texName (ID number) to use for this texture
	glGenTextures( 1, (GLuint*)&m_openglTextureID );

	// Tell OpenGL to bind (set) this as the currently active texture
	glBindTexture( GL_TEXTURE_2D, m_openglTextureID );

	// Set texture clamp vs. wrap (repeat)
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP ); // one of: GL_CLAMP or GL_REPEAT
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP ); // one of: GL_CLAMP or GL_REPEAT

	// Set magnification (texel > pixel) and minification (texel < pixel) filters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );  // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR

	GLenum bufferFormat = GL_RGBA; // the format our source pixel data is currently in; any of: GL_RGB, GL_Rgba, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
	if ( numComponents == 3 )
		bufferFormat = GL_RGB;

	// #FIXME: What happens if numComponents is neither 3 nor 4?

	GLenum internalFormat = bufferFormat; // the format we want the texture to me on the card; allows us to translate into a different texture format as we upload to OpenGL

	glTexImage2D(			// Upload this pixel data to our new OpenGL texture
				  GL_TEXTURE_2D,		// Creating this as a 2d texture
				  0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
				  internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
				  m_sizeInTexels.x,		// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,10], and B is the border thickness [0,1]
				  m_sizeInTexels.y,		// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,10], and B is the border thickness [0,1]
				  0,					// Border size, in texels (must be 0 or 1)
				  bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
				  GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color/alpha channel)
				  imageData );		// Location of the actual pixel data bytes/buffer

	glDisable( GL_TEXTURE_2D );
}


//---------------------------------------------------------------------------
Texture::Texture( const std::string& textureName, unsigned int width, unsigned int height, TextureFormat format )
	: m_sizeInTexels( width, height )
	, m_filepath( textureName )
{
	GLuint textureType = GL_TEXTURE_2D;

	glGenTextures( 1, &m_openglTextureID );

	//glTexParameteri: not used beyond OpenGL 1.0 because samplers get used for those rules!

	GLenum bufferChannels = GL_RGBA;
	GLenum bufferFormat = GL_UNSIGNED_INT_8_8_8_8; //how channels are laid out.
	GLenum internalFormat = GL_RGBA8;

	if ( format == TEXTURE_FORMAT_Rgba8 )
	{
		//Nothing changes, our above defaults are fine.
	}
	else if ( format == TEXTURE_FORMAT_Depth24_Stencil8 )
	{
		bufferChannels = GL_DEPTH_STENCIL;
		bufferFormat = GL_UNSIGNED_INT_24_8;
		internalFormat = GL_DEPTH24_STENCIL8;
	}
	else
	{
		ERROR_AND_DIE( "Found unsupported TextureFormat in Texture()!" );
	}

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( textureType, m_openglTextureID ); //Can store it off as well!
	glTexImage2D( textureType, //What bind point you're generating the image on
		0, // mipmap level
		internalFormat, //how tex is stored in memory
		width,
		height,
		0, // border, again set to 0, we want not 0
		bufferChannels, // channels used by image passed in
		bufferFormat, // format of data of image passed in
		NULL // no actual data passed in! Defaults to black. Maybe white.
	);
}


//---------------------------------------------------------------------------
// Returns a pointer to the already-loaded texture of a given image file,
//	or nullptr if no such texture/image has been loaded.
//
STATIC Texture* Texture::GetTextureByPath( const std::string& imageFilePath )
{
	size_t pathHash = std::hash<std::string>{}( imageFilePath );
	if ( s_textureRegistry.find( pathHash ) == s_textureRegistry.end() ) return nullptr;
	else return s_textureRegistry[ pathHash ];
}


//---------------------------------------------------------------------------
Texture* Texture::CreateTextureFromBytes( const std::string& textureName, const unsigned char* imageData, const Vector2i& textureSize, unsigned int numComponents )
{
	size_t nameHash = std::hash<std::string>{}( textureName );
	if ( s_textureRegistry.count( nameHash ) > 0 ) return s_textureRegistry[ nameHash ];
	else
	{
		s_textureRegistry[ nameHash ] = new Texture( textureName, imageData, textureSize, numComponents );
		return s_textureRegistry[ nameHash ];
	}
}


//--------------------------------------------------------------------------------------------------------------
STATIC Texture* Texture::CreateTextureFromNoData( const std::string& textureName, unsigned int width, unsigned int height, TextureFormat format )
{
	size_t nameHash = std::hash<std::string>{}( textureName );
	if ( s_textureRegistry.count( nameHash ) > 0 ) return s_textureRegistry[ nameHash ];
	else
	{
		s_textureRegistry[ nameHash ] = new Texture( textureName, width, height, format );
		return s_textureRegistry[ nameHash ];
	}
}


//--------------------------------------------------------------------------------------------------------------
// Finds the named Texture among the registry of those already loaded; if
//	found, returns that Texture*.  If not, attempts to load that texture,
//	and returns a Texture* just created (or nullptr if unable to load file).
//
STATIC Texture* Texture::CreateOrGetTexture( const std::string& imageFilePath )
{
	size_t pathHash = std::hash<std::string>{}( imageFilePath );
	if ( s_textureRegistry.find( pathHash ) != s_textureRegistry.end() )
		return s_textureRegistry[ pathHash ];

	FILE *f = fopen( imageFilePath.c_str(), "rb" );

	if ( f == nullptr ) 
		return nullptr;
	else 
		s_textureRegistry[ pathHash ] = new Texture( imageFilePath );

	fclose( f );

	return s_textureRegistry[ pathHash ];
}

