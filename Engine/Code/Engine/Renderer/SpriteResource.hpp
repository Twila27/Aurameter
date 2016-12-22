#pragma once

#include "Engine/EngineCommon.hpp"
#include "Engine/Memory/UntrackedAllocator.hpp"
#include <utility>
class Texture;
class Material;
struct XMLNode;


//-----------------------------------------------------------------------------
class SpriteResource;
typedef std::vector< SpriteResource*, UntrackedAllocator<SpriteResource*> > SpriteResourceRegistryVector;


//-----------------------------------------------------------------------------
class SpriteResource
{
public:
	static SpriteResource* Create( ResourceID id, const char* textureFilename );
	void WriteToXMLNode( XMLNode& resourceNode );

	ResourceID GetID() const { return m_id; }

	Material* GetMaterial() const { return m_defaultMaterial; }
	unsigned int GetDiffuseTextureID() const;
	Texture* GetDiffuseTexture() const;

	void SetDiffuseTexture( const std::string& newTexFilePath );
	void SetMaterial( Material* newMat ) { m_defaultMaterial = newMat; }
	void SetUV( const Vector2f& mins, const Vector2f& maxs );

private:
	SpriteResource() { m_diffuseTexture = nullptr; m_defaultMaterial = nullptr; }
	SpriteResource( const SpriteResource& ) {} //Disallow copying.
	void operator=( const SpriteResource& ) {} //Disallow assignment/pass by value.
	ResourceID m_id;

	//Change below two members to be a list if looking to extend sprites to have normal maps, etc.
	Texture* m_diffuseTexture;
	AABB2f m_texCoords;

	Material* m_defaultMaterial; //i.e. can be overridden by Sprite's member.
};
