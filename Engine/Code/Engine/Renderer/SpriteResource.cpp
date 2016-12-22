#include "Engine/Renderer/SpriteResource.hpp"

#include "Engine/Renderer/ResourceDatabase.hpp"
#include "Engine/Renderer/SpriteRenderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"
#include "Engine/Renderer/Material.hpp"


//--------------------------------------------------------------------------------------------------------------
SpriteResource* SpriteResource::Create( ResourceID id, const char* textureFilename )
{
	Texture* diffuseTex = Texture::CreateOrGetTexture( textureFilename );
	ASSERT_RETURN( diffuseTex );

	ASSERT_OR_DIE( ResourceDatabase::Count( id ) == 0, "Non-unique SpriteResource!" );

	SpriteResource* sr = new SpriteResource();
	sr->m_id = id;
	sr->m_diffuseTexture = diffuseTex;
	sr->m_defaultMaterial = SpriteRenderer::s_defaultSpriteMaterial;
		//Recommends typing this up in a .vert like SD3, but then just pasting that and hard-loading it 
		//with no dependencies using the CompileShader( const char* sourceCodeBuffer ).
		//Not your pink-and-black colored checkerboard, just a simple diffuse material that can be used for very basic objects, like Unity's primitives.
	
	//For now, just show entire sprite:
	sr->m_texCoords = AABB2f( 0.f, 0.f, 1.f, 1.f );

	ResourceDatabase::Instance()->AddSpriteResource( id, sr );

	return sr;
}


//--------------------------------------------------------------------------------------------------------------
void SpriteResource::WriteToXMLNode( XMLNode& resourceNode )
{
	WriteXMLAttribute( resourceNode, "name",			m_id, std::string() );
	WriteXMLAttribute( resourceNode, "image",			m_diffuseTexture->GetFilePath(), std::string() );

	std::string materialName;
	if ( m_defaultMaterial != nullptr )
		materialName = m_defaultMaterial->GetName();

	WriteXMLAttribute( resourceNode, "material",		materialName, std::string() );
	WriteXMLAttribute( resourceNode, "uvTopLeft",		Stringf( "%.4f,%.4f", m_texCoords.mins.x, m_texCoords.mins.y ), std::string( "0,0" ) );
	WriteXMLAttribute( resourceNode, "uvBottomRight",	Stringf( "%.4f,%.4f", m_texCoords.maxs.x, m_texCoords.maxs.y ), std::string( "1,1" ) );
}


//--------------------------------------------------------------------------------------------------------------
unsigned int SpriteResource::GetDiffuseTextureID() const
{
	return m_diffuseTexture->GetTextureID();
}



//--------------------------------------------------------------------------------------------------------------
Texture* SpriteResource::GetDiffuseTexture() const
{
	return m_diffuseTexture;
}


//--------------------------------------------------------------------------------------------------------------
void SpriteResource::SetDiffuseTexture( const std::string& newTexFilePath )
{
	Texture* diffuseTex = Texture::CreateOrGetTexture( newTexFilePath );
	ASSERT_RETURN( diffuseTex );

	m_diffuseTexture = diffuseTex;
}


//--------------------------------------------------------------------------------------------------------------
void SpriteResource::SetUV( const Vector2f& mins, const Vector2f& maxs )
{
	m_texCoords = AABB2f( mins, maxs );
}
