#include "Engine/Renderer/ResourceDatabase.hpp"
#include "Engine/EngineCommon.hpp"
#include "Engine/Renderer/SpriteResource.hpp"
#include "Engine/Renderer/SpriteRenderer.hpp"
#include "Engine/Core/TheConsole.hpp"
#include "Engine/FileUtils/FileUtils.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"

#include "Engine/Renderer/AnimatedSprite.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC SpriteResourceRegistryMap ResourceDatabase::s_spriteResourceRegistry;
STATIC SpriteSequenceRegistryMap ResourceDatabase::s_spriteSequenceRegistry;
STATIC ParticleEmitterDefinitionRegistryMap ResourceDatabase::s_particleEmitterRegistry;
STATIC ParticleSystemDefinitionRegistryMap ResourceDatabase::s_particleSystemRegistry;
STATIC ResourceDatabase* ResourceDatabase::s_theResourceDatabase = nullptr;


//--------------------------------------------------------------------------------------------------------------
STATIC ResourceDatabase* /*CreateOrGet*/ ResourceDatabase::Instance()
{
	if ( s_theResourceDatabase == nullptr )
		s_theResourceDatabase = new ResourceDatabase();

	return s_theResourceDatabase;
}


//--------------------------------------------------------------------------------------------------------------
static SpriteResource* CreateSpriteResourceFromXML( const XMLNode& resourceNode )
{
	std::string name, imageFilePath, materialName, uvTopLeftStr, uvBottomRightStr;
	Vector2f uv_tl = Vector2f::ZERO;
	Vector2f uv_br = Vector2f::ONE;

	name = ReadXMLAttribute( resourceNode, "name", name );
	imageFilePath = ReadXMLAttribute( resourceNode, "image", imageFilePath );
	materialName = ReadXMLAttribute( resourceNode, "material", materialName );
	uvTopLeftStr = ReadXMLAttribute( resourceNode, "uvTopLeft", uvTopLeftStr );
	uvBottomRightStr = ReadXMLAttribute( resourceNode, "uvBottomRight", uvBottomRightStr );

	sscanf_s( uvTopLeftStr.c_str(), "%f,%f", &uv_tl.x, &uv_tl.y );
	sscanf_s( uvBottomRightStr.c_str(), "%f,%f", &uv_br.x, &uv_br.y );

	SpriteResource* sr = SpriteResource::Create( name, imageFilePath.c_str() ); //Will add to DB in here.

	if ( materialName != "" )
	{
		ShaderProgram* sp = ShaderProgram::CreateOrGetShaderProgram( materialName );
		sr->SetMaterial( Material::CreateOrGetMaterial( materialName, &SpriteRenderer::s_defaultSpriteRenderState, &sp->GetVertexDefinition(), materialName.c_str() ) );
	}
	else sr->SetMaterial( nullptr );

	sr->SetUV( uv_tl, uv_br );

	return sr;
}


//--------------------------------------------------------------------------------------------------------------
static void CreateSpriteSequenceFromXML( const XMLNode& animNode )
{
	std::string name = "Unnamed Animation";
	std::string loopModeStr;
	float durationSeconds = -1.f;

	name = ReadXMLAttribute( animNode, "name", name );
	loopModeStr = ReadXMLAttribute( animNode, "loopMode", loopModeStr );
	durationSeconds = ReadXMLAttribute( animNode, "durationSeconds", durationSeconds );

	AnimatedSpriteSequence* anim = new AnimatedSpriteSequence( name, GetLoopModeForString( loopModeStr ), durationSeconds );

	//Note keyframes will have their own SpriteResources registered to the database.
	for ( int keyframeIndex = 0; keyframeIndex < animNode.nChildNode(); keyframeIndex++ )
	{
		XMLNode keyframeNode = animNode.getChildNode( keyframeIndex );
		SpriteResource* sr = CreateSpriteResourceFromXML( keyframeNode );
		float keyframeStartSeconds = -1.f;
		keyframeStartSeconds = ReadXMLAttribute( keyframeNode, "keyframeStart", keyframeStartSeconds );
		anim->AddKeyframe( keyframeStartSeconds, sr );
	}

	ResourceDatabase::Instance()->AddSpriteSequence( name, anim );
}


//--------------------------------------------------------------------------------------------------------------
ResourceType GetResourceTypeForString( const std::string& typeName )
{
	std::string loweredName = GetAsLowercase( typeName ).c_str();

	if ( loweredName == "sprite" )
		return RESOURCE_TYPE_SPRITE;
	if ( loweredName == "animatedsprite" || loweredName == "spriteanimation" )
		return RESOURCE_TYPE_SPRITE_ANIMATION;
	if ( loweredName == "particlesystem" )
		return RESOURCE_TYPE_PARTICLE_SYSTEM;
	if ( loweredName == "particleemitter" )
		return RESOURCE_TYPE_PARTICLE_EMITTER;

	ERROR_RECOVERABLE( "Unexpected value in GetResourceTypeForString!" );
	return NUM_RESOURCE_TYPES;
}


//--------------------------------------------------------------------------------------------------------------
std::string GetStringForResourceType( ResourceType type )
{
	switch ( type )
	{
	case RESOURCE_TYPE_SPRITE: return "sprite";
	case RESOURCE_TYPE_SPRITE_ANIMATION: return "animatedsprite";
	case RESOURCE_TYPE_PARTICLE_SYSTEM: return "particlesystem";
	case RESOURCE_TYPE_PARTICLE_EMITTER: return "particleemitter";
	default:
		ERROR_RECOVERABLE( "Unexpected value in GetStringForResourceType!" );
		return "";
	}

}


//--------------------------------------------------------------------------------------------------------------
STATIC void ResourceDatabase::LoadAll()
{
	ResourceDatabase::LoadAllSpriteResources();
}


//--------------------------------------------------------------------------------------------------------------
STATIC void ResourceDatabase::LoadAllSpriteResources()
{
	//Note we may have more than one SpriteResource in a file.
	std::vector< std::string > m_spriteFiles = EnumerateFilesInDirectory( "Data/XML/SpriteResources", "*.Sprites.xml" );
	
	for ( unsigned int spriteFileIndex = 0; spriteFileIndex < m_spriteFiles.size(); spriteFileIndex++ )
	{
		const char* xmlFilename = m_spriteFiles[ spriteFileIndex ].c_str();
		XMLNode resourcesRoot = XMLNode::openFileHelper( xmlFilename, "SpriteResources" );

		for ( int resourceIndex = 0; resourceIndex < resourcesRoot.nChildNode(); resourceIndex++ )
		{
			XMLNode resourceNode = resourcesRoot.getChildNode( resourceIndex );

			if ( strcmp( resourceNode.getName(), "SpriteAnimation" ) == 0 )
				CreateSpriteSequenceFromXML( resourceNode );
			else //Normal 1-sprite SpriteResource
				CreateSpriteResourceFromXML( resourceNode );
				
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
STATIC void ResourceDatabase::ReloadAllSpriteResources()
{	
	//Note we may have more than one SpriteResource in a file.
	std::vector< std::string > m_spriteFiles = EnumerateFilesInDirectory( "Data/XML/SpriteResources", "*.Sprites.xml" );

	for ( unsigned int spriteFileIndex = 0; spriteFileIndex < m_spriteFiles.size(); spriteFileIndex++ )
	{
		const char* xmlFilename = m_spriteFiles[ spriteFileIndex ].c_str();
		XMLNode resourcesRoot = XMLNode::openFileHelper( xmlFilename, "SpriteResources" );

		for ( int resourceIndex = 0; resourceIndex < resourcesRoot.nChildNode(); resourceIndex++ )
		{
			XMLNode resourceNode = resourcesRoot.getChildNode( resourceIndex );

			std::string name, imageFilePath, materialName, uvTopLeftStr, uvBottomRightStr;
			Vector2f uv_tl = Vector2f::ZERO;
			Vector2f uv_br = Vector2f::ONE;

			name = ReadXMLAttribute( resourceNode, "name", name );
			imageFilePath = ReadXMLAttribute( resourceNode, "image", imageFilePath );
			materialName = ReadXMLAttribute( resourceNode, "material", materialName );
			uvTopLeftStr = ReadXMLAttribute( resourceNode, "uvTopLeft", uvTopLeftStr );
			uvBottomRightStr = ReadXMLAttribute( resourceNode, "uvBottomRight", uvBottomRightStr );

			sscanf_s( uvTopLeftStr.c_str(), "%f,%f", &uv_tl.x, &uv_tl.y );
			sscanf_s( uvBottomRightStr.c_str(), "%f,%f", &uv_br.x, &uv_br.y );

			SpriteResource* sr;
			if ( s_spriteResourceRegistry.count( name ) > 0 )
			{
				sr = s_spriteResourceRegistry.at( name );
				sr->SetDiffuseTexture( imageFilePath );
				//sr->SetMaterial(); //If we get to where we can read that in from XML via the AES supplementary tutorials.
			}
			else
			{
				sr = SpriteResource::Create( name, imageFilePath.c_str() ); //Will add to DB in here.
			}

			if ( materialName != "" )
			{
				ShaderProgram* sp = ShaderProgram::CreateOrGetShaderProgram( materialName );
				sr->SetMaterial( Material::CreateOrGetMaterial( materialName, &SpriteRenderer::s_defaultSpriteRenderState, &sp->GetVertexDefinition(), materialName.c_str() ) );
			}
			else sr->SetMaterial( nullptr );

			sr->SetUV( uv_tl, uv_br );
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
void ResourceDatabase::WriteSpritesToFile( const std::string& filename )
{
	XMLNode resourcesNode = XMLNode::createXMLTopNode( "SpriteResources" );

	for ( const SpriteResourceRegistryPair& resPair : s_spriteResourceRegistry )
	{
		XMLNode resourceNode = resourcesNode.addChild( "SpriteResource" );
		resPair.second->WriteToXMLNode( resourceNode );
	}

	std::string saveFilename = Stringf( "Data/XML/SpriteResources/%s.Sprites.xml", filename.c_str() );
	resourcesNode.writeToFile( saveFilename.c_str() );

	g_theConsole->Printf( "Saved to %s", saveFilename.c_str() );
}
