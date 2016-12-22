#pragma once


#include <map>

#include "Engine/Math/Vector2.hpp"
#include "Engine/Memory/UntrackedAllocator.hpp"


//--------------------------------------------------------------------------------------------------------------
enum TextureFormat
{
	TEXTURE_FORMAT_Rgba8, //Rgba, 8 Bits Per Channel.
	TEXTURE_FORMAT_Depth24_Stencil8  //Depth 24-bits, Stencil 8-bits.
};


//--------------------------------------------------------------------------------------------------------------
class Texture;
typedef std::pair< size_t, Texture* > TextureRegistryPair;
typedef std::map< size_t, Texture*, std::less<size_t>, UntrackedAllocator<TextureRegistryPair> > TextureRegistryMap;


//--------------------------------------------------------------------------------------------------------------
class Texture
{
public:
	static Texture* CreateOrGetTexture( const std::string& imageFilePath );
	static const TextureRegistryMap& GetRegistry() { return s_textureRegistry; }

	std::string GetFilePath() { return m_filepath; }
	static Texture* GetTextureByPath( const std::string& imageFilePath );
	unsigned int GetTextureID() const { return m_openglTextureID; }
	unsigned int GetWidth() const { return m_sizeInTexels.x; }
	unsigned int GetHeight() const { return m_sizeInTexels.y; }
	Vector2i GetTextureDimensions() const { return m_sizeInTexels; }
	static Texture* CreateTextureFromBytes( const std::string& textureName, const unsigned char* imageData, const Vector2i& textureSizeInTexels, unsigned int numComponents ); //Args from an stbi_load() prior to this call. 
	static Texture* CreateTextureFromNoData( const std::string& textureName, unsigned int width, unsigned int height, TextureFormat format ); //SD3, for FBOs.

private:
	Texture( const std::string& imageFilePath );
	Texture( const std::string& textureName, const unsigned char* imageData, const Vector2i& textureSizeInTexels, unsigned int numComponents ); //Called by CreateFromBytes.
	Texture( const std::string& textureName, unsigned int width, unsigned int height, TextureFormat format ); //Called by CreateFromNoData.

	std::string m_filepath;
	static TextureRegistryMap s_textureRegistry;
	unsigned int m_openglTextureID;
	Vector2i m_sizeInTexels;
};