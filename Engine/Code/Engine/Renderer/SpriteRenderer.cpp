#include "Engine/Renderer/SpriteRenderer.hpp"
#include "Engine/Renderer/ResourceDatabase.hpp"
#include "Engine/Renderer/TheRenderer.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Core/TheConsole.hpp"
#include "Engine/Core/Command.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/EngineCommon.hpp"
#include "Engine/Math/Matrix4x4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Camera2D.hpp"
#include "Engine/Renderer/FrameBuffer.hpp"
#include "Engine/Renderer/FrameBufferEffect.hpp"
#include "Engine/Renderer/RiftUtils.hpp"
#include "Engine/Renderer/Particles/ParticleSystem.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC SpriteLayerRegistryMap SpriteRenderer::s_spriteLayers = SpriteLayerRegistryMap();
STATIC Material* SpriteRenderer::s_defaultSpriteMaterial = nullptr;
STATIC std::shared_ptr<Mesh> SpriteRenderer::s_spriteMesh = nullptr;
STATIC MeshRenderer* SpriteRenderer::s_spriteMeshRenderer = nullptr;
STATIC const RenderState SpriteRenderer::s_defaultSpriteRenderState = RenderState( CULL_MODE_NONE/*FOR_VR*/, BLEND_MODE_SOURCE_ALPHA, BLEND_MODE_ONE_MINUS_SOURCE_ALPHA, DEPTH_COMPARE_MODE_LESS, false );
STATIC Rgba SpriteRenderer::s_clearColor = Rgba::WHITE;
STATIC bool SpriteRenderer::s_shouldHide2D = false;
STATIC bool SpriteRenderer::s_shouldHide3D = true;
STATIC bool SpriteRenderer::s_shouldCull = false;
STATIC bool SpriteRenderer::s_isParentingEnabled = false;
STATIC unsigned int SpriteRenderer::s_numSpritesCulled = 0;
STATIC FrameBuffer* SpriteRenderer::s_currentRenderTarget = nullptr;
STATIC FrameBuffer* SpriteRenderer::s_effectRenderTarget = nullptr;

STATIC float SpriteRenderer::s_aspectRatio;
STATIC float SpriteRenderer::s_defaultImportSize;
STATIC Vector2f SpriteRenderer::s_virtualScreenSize;
STATIC Vector2f SpriteRenderer::s_defaultVirtualSize;
STATIC const Camera2D* SpriteRenderer::s_activeCamera;


//--------------------------------------------------------------------------------------------------------------
static const char* defaultVertexShaderSource = "\
#version 410 core\n\
\n\
//Recall, uniforms are 'uniform' across all vertices, where in-variables are unique per-vertex. \n\
uniform mat4 uProj; //Will end up being our ortho, but still a proj matrix. \n\
uniform mat4 uView; //NO uModel because the sprites will be in their world positions already when we make the meshes. \n\
					//Since we're making these meshes each frame in world space.\n\
					//Very typical for a 2D sprite system or particle system.\n\
\n\
in vec2 inPosition;\n\
in vec2 inUV0;\n\
in vec4 inColor;\n\
\n\
out vec2 passthroughUV0;\n\
out vec4 passthroughTintColor;\n\
\n\
void main()\n\
{\n\
	passthroughUV0 = inUV0;\n\
	passthroughTintColor = inColor;\n\
\n\
	vec4 pos = vec4( inPosition, 0, 1 ); //1 to preserve translation, 0 to ignore depth.\n\
										 //Could sort by using a value other than 0 for z, rather than sorting by layers.\n\
										 //This would use the depth buffer, but encounters issues of z-fighting, i.e. layers flickering back and forth, whereas painters always just splats one onto the other.\n\
	gl_Position = pos * uView * uProj;\n\
}";


//--------------------------------------------------------------------------------------------------------------
static const char* defaultFragmentShaderSource = "\n\
#version 410 core\n\
\n\
//A layer tint to tint the entire layer could be sent as a uniform, e.g. fade out the UI and game layers on pause.\n\
//Noted that layer effects would all be shaders.\n\
uniform sampler2D uTexDiffuse;\n\
\n\
in vec2 passthroughUV0;\n\
in vec4 passthroughTintColor;\n\
\n\
out vec4 outColor;\n\
\n\
void main()\n\
{\n\
	vec4 diffuseTexColor = texture( uTexDiffuse, passthroughUV0 );\n\
	outColor = diffuseTexColor * passthroughTintColor;\n\
}";


//--------------------------------------------------------------------------------------------------------------
void SpriteRenderer::DisableLayer( Command& arg )
{
	int outLayerID;
	bool doesArgExist = arg.GetNextInt( &outLayerID, 0 );
	if ( doesArgExist )
	{
		SpriteRenderer::DisableLayer( outLayerID );
	}
	else
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SpriteRendererDisableLayer <intLayerID>" );
		PrintLayers( arg );
	}
}


//--------------------------------------------------------------------------------------------------------------
void SpriteRenderer::EnableLayer( Command& arg )
{
	int outLayerID;
	bool doesArgExist = arg.GetNextInt( &outLayerID, 0 );
	if ( doesArgExist )
	{
		SpriteRenderer::EnableLayer( outLayerID );
	}
	else
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SpriteRendererEnableLayer <intLayerID>" );
		PrintLayers( arg );
	}
}


//--------------------------------------------------------------------------------------------------------------
void SpriteRenderer::ToggleLayer( Command& arg )
{
	int outLayerID;
	bool doesArgExist = arg.GetNextInt( &outLayerID, 0 );
	if ( doesArgExist )
	{
		SpriteRenderer::ToggleLayer( outLayerID );
	}
	else
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SpriteRendererToggleLayer <intLayerID>" );
		PrintLayers( arg );
	}
}


//--------------------------------------------------------------------------------------------------------------
void SpriteRenderer::PrintLayers( Command& )
{
	g_theConsole->Printf( "Current layer IDs:" );
	SpriteLayerRegistryMap::const_iterator layerIterEnd = s_spriteLayers.cend();
	for ( SpriteLayerRegistryMap::const_iterator layerIter = s_spriteLayers.cbegin(); layerIter != layerIterEnd; ++layerIter )
	{
		const RenderLayer* layer = layerIter->second;
		const char* enabledStr = ( layer->m_enabled ? "True" : "False" );
		g_theConsole->Printf( "ID: %d  Enabled: %s    Name: %s", layer->m_layerID, enabledStr, layer->m_name.c_str() );
	}
}


//--------------------------------------------------------------------------------------------------------------
void SpriteRenderer::CreateOrRenameLayer( Command& args )
{

	int outLayerID;
	bool doesArgExist = args.GetNextInt( &outLayerID, 0 );
	if ( doesArgExist )
	{
		std::string outLayerName;
		doesArgExist = args.GetNextString( &outLayerName, 0 );
		if ( doesArgExist )
		{
			if ( s_spriteLayers.count( outLayerID ) > 0 )
				s_spriteLayers[ outLayerID ]->m_name = outLayerName;
			else
				s_spriteLayers[ outLayerID ] = new RenderLayer( outLayerID, outLayerName );
		}
		else
		{
			g_theConsole->Printf( "Incorrect arguments: did not find a new layer name supplied second." );
			g_theConsole->Printf( "Usage: SpriteRendererNameLayer <intLayerID> <stringNewLayerName>" );
		}
	}
	else
	{
		g_theConsole->Printf( "Incorrect arguments: did not find ID first." );
		g_theConsole->Printf( "Usage: SpriteRendererNameLayer <intLayerID> <stringNewLayerName>" );
		PrintLayers( args );
	}
}


//--------------------------------------------------------------------------------------------------------------
static void RegisterConsoleCommands()
{
	g_theConsole->RegisterCommand( "SpriteRendererToggleSpriteParenting", SpriteRenderer::ToggleSpriteParenting );
	g_theConsole->RegisterCommand( "SpriteRendererSaveAllSprites", SpriteRenderer::SaveAllSprites );
	g_theConsole->RegisterCommand( "SpriteRendererReloadAllSprites", SpriteRenderer::ReloadAllSprites );
	g_theConsole->RegisterCommand( "SpriteRendererListLayers", SpriteRenderer::PrintLayers );
	g_theConsole->RegisterCommand( "SpriteRendererPrintLayers", SpriteRenderer::PrintLayers );
	g_theConsole->RegisterCommand( "SpriteRendererCreateOrRenameLayer", SpriteRenderer::CreateOrRenameLayer );
	g_theConsole->RegisterCommand( "SpriteRendererDisableLayer", SpriteRenderer::DisableLayer );
	g_theConsole->RegisterCommand( "SpriteRendererEnableLayer", SpriteRenderer::EnableLayer );
	g_theConsole->RegisterCommand( "SpriteRendererToggleLayer", SpriteRenderer::ToggleLayer );
	g_theConsole->RegisterCommand( "ToggleShowing3D", SpriteRenderer::ToggleShowing3D );
	g_theConsole->RegisterCommand( "ToggleShowing2D", SpriteRenderer::ToggleShowing2D );
	g_theConsole->RegisterCommand( "SpriteRendererToggleCulling", SpriteRenderer::ToggleCulling );
}


//--------------------------------------------------------------------------------------------------------------
void SpriteRenderer::Startup( const Camera2D* activeCamera, const Rgba& clearColor, bool shouldHide3D )
{
	RegisterConsoleCommands();
	s_shouldHide3D = shouldHide3D;
	s_clearColor = clearColor;
	s_activeCamera = activeCamera;

	//Make FBO.
	std::vector<TextureFormat> colorFormats;
	colorFormats.push_back( TextureFormat::TEXTURE_FORMAT_Rgba8 );
	const TextureFormat depthStencilFormat1 = TextureFormat::TEXTURE_FORMAT_Depth24_Stencil8; //Else deleted twice because we're passing its address.
	const TextureFormat depthStencilFormat2 = TextureFormat::TEXTURE_FORMAT_Depth24_Stencil8;

	//Should they have separate 3rd and 4th args?
	s_currentRenderTarget = new FrameBuffer( (unsigned int)g_theRenderer->GetScreenWidth(), (unsigned int)g_theRenderer->GetScreenHeight(), colorFormats, &depthStencilFormat1 );
	s_effectRenderTarget = new FrameBuffer( (unsigned int)g_theRenderer->GetScreenWidth(), (unsigned int)g_theRenderer->GetScreenHeight(), colorFormats, &depthStencilFormat2 );

	//Taken from TheRenderer::m_defaultFboQuad code, but position will be constantly rewritten by SpriteRenderer::SpriteRender().
	Vertex2D_PCT quad[ 4 ] =
	{
		Vertex2D_PCT( Vector2f::ZERO, Rgba::WHITE, Vector2f( 1.f, 1.f ) ), //Top-left.	 //0
		Vertex2D_PCT( Vector2f::ZERO, Rgba::WHITE, Vector2f( 0.f, 0.f ) ), //Bottom-right. //1
		Vertex2D_PCT( Vector2f::ZERO, Rgba::WHITE, Vector2f( 0.f, 1.f ) ), //Bottom-left.	 //2
		Vertex2D_PCT( Vector2f::ZERO, Rgba::WHITE, Vector2f( 1.f, 0.f ) ) //Top-right.	 //3
	};
	unsigned int quadIndices[] =
	{
		2, 1, 0, //Counter-Clockwise, else renders to the back and the quad won't show with backface culling.
		0, 1, 3
	};

	DrawInstruction quadDrawInstructions[] = { DrawInstruction( VertexGroupingRule::AS_TRIANGLES, 0, 6, true ) };
	Mesh* spriteMesh = new Mesh( BufferUsage::STATIC_DRAW, Vertex2D_PCT::DEFINITION, _countof( quad ), quad, _countof( quadIndices ), quadIndices, 1, quadDrawInstructions );

	s_spriteMesh = std::shared_ptr<Mesh>( spriteMesh );

	Shader* defaultVertexShader = Shader::CreateShaderFromSource( defaultVertexShaderSource, strlen( defaultVertexShaderSource ), ShaderType::VERTEX_SHADER );
	Shader* defaultFragmentShader = Shader::CreateShaderFromSource( defaultFragmentShaderSource, strlen( defaultFragmentShaderSource ), ShaderType::FRAGMENT_SHADER );

	s_defaultSpriteMaterial = Material::CreateOrGetMaterial( "BasicSprite", &s_defaultSpriteRenderState, &Vertex2D_PCT::DEFINITION, "BasicSprite", defaultVertexShader, defaultFragmentShader );
	s_defaultSpriteMaterial->SetSampler( "uTexDiffuse", TheRenderer::DEFAULT_SAMPLER_ID );
	s_defaultSpriteMaterial->SetMatrix4x4( "uView", false, &Matrix4x4f::IDENTITY );
	s_spriteMeshRenderer = new MeshRenderer( s_spriteMesh, s_defaultSpriteMaterial );

	SpriteResource::Create( "Default", g_theRenderer->GetDefaultTexture()->GetFilePath().c_str() );
}


//--------------------------------------------------------------------------------------------------------------
void SpriteRenderer::LoadAllSpriteResources()
{
	ResourceDatabase::LoadAllSpriteResources();
}


//--------------------------------------------------------------------------------------------------------------
void SpriteRenderer::ReloadAllSprites( Command& )
{
	SpriteRenderer::ReloadAllSprites();
}


//--------------------------------------------------------------------------------------------------------------
void SpriteRenderer::ReloadAllSprites()
{
	ResourceDatabase::ReloadAllSpriteResources();

	for ( const SpriteLayerRegistryPair& layerPair : s_spriteLayers )
		for ( Sprite* sprite : layerPair.second->m_sprites )
			SpriteRenderer::ResizeSprite( sprite );
}


//--------------------------------------------------------------------------------------------------------------
void SpriteRenderer::SaveAllSprites( Command& arg )
{
	std::string outFilename;
	bool doesArgExist = arg.GetNextString( &outFilename );
	if ( doesArgExist )
	{
		ResourceDatabase::WriteSpritesToFile( outFilename );
		g_theConsole->Printf( "WriteSpritesToFile completed." );
	}
	else
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SpriteRendererSaveAllSprites <stringOutFilename>" );
		g_theConsole->Printf( "The code will prepend Data/XML/SpriteResources and append .Sprites.xml." );
	}
}


//--------------------------------------------------------------------------------------------------------------
void SpriteRenderer::Shutdown()
{
	ResourceDatabase::Shutdown();

	//Material cleaned up by its registry.
	if ( s_spriteMeshRenderer != nullptr )
	{
		delete s_spriteMeshRenderer;
		s_spriteMeshRenderer = nullptr;
	}

	if ( s_spriteLayers.size() > 0 )
	{
		for ( const SpriteLayerRegistryPair& pair : s_spriteLayers )
		{
			if ( pair.second != nullptr )
				delete pair.second;
		}
		s_spriteLayers.clear();
	}

	if ( s_currentRenderTarget != nullptr )
		delete s_currentRenderTarget;
	if ( s_effectRenderTarget != nullptr )
		delete s_effectRenderTarget;
}


//--------------------------------------------------------------------------------------------------------------
unsigned int SpriteRenderer::GetNumLiveParticles()
{
	unsigned int total = 0;
	for ( const SpriteLayerRegistryPair& pair : s_spriteLayers )
	{
		for ( ParticleSystem* ps : pair.second->m_particleSystems )
			total += ps->GetNumLiveParticles();
	}

	return total;
}


//--------------------------------------------------------------------------------------------------------------
void SpriteRenderer::UpdateAspectRatio( float windowWidthPx, float windowHeightPx )
{
	s_aspectRatio = windowWidthPx / windowHeightPx;
	s_virtualScreenSize = s_defaultVirtualSize; //So the below overwrite leaves the other one small.

	if ( s_aspectRatio > 1 ) //horizontal monitor, since width > height.
		s_virtualScreenSize.x = s_defaultVirtualSize.x * s_aspectRatio; //so height stays constant. Cancels out the px.
	else //vertical monitor.
		s_virtualScreenSize.y = s_defaultVirtualSize.y * s_aspectRatio; //so width stays constant. Cancels out the px.

	//For example: if we want a virtual square of 10x10, but 16:10 is our aspect ratio > 1, we expand vwidth := 10*16/10 = 16.
}


//--------------------------------------------------------------------------------------------------------------
STATIC void SpriteRenderer::RenderFrame()
{
	if ( s_shouldHide3D )
		g_theRenderer->ClearScreenToColor( s_clearColor ); //Cover up the engine's preceding 3D calls.

	if ( s_shouldHide2D )
		return;

//	g_theRenderer->BindFBO( s_currentRenderTarget );

	for ( const SpriteLayerRegistryPair& layerPair : s_spriteLayers )
	{
		RenderLayer* layer = layerPair.second;
		DrawLayer( layer );
	}
}


//--------------------------------------------------------------------------------------------------------------
void SpriteRenderer::DrawLayer( RenderLayer* layer )
{
	if ( !layer->m_enabled )
		return;

	for ( Sprite* sprite : layer->m_sprites )
		SpriteRenderer::RenderSprite( sprite, layer->IsScrolling() );

	for ( ParticleSystem* particleSystem : layer->m_particleSystems )
		particleSystem->Render();

	for ( FramebufferEffect* fboEffect : layer->m_effects )
	{
//		g_theRenderer->BindFBO( s_effectRenderTarget ); //Render current render target to effect render target.

		fboEffect->m_fboEffectRenderer->SetTexture( "uTexDiffuse", s_currentRenderTarget->GetColorTextureID( 0 ) );

//		fboEffect->m_fboEffectRenderer->Render();

		//FrameBuffer* temp = s_effectRenderTarget;
		//s_effectRenderTarget = s_currentRenderTarget;
		//s_currentRenderTarget = temp;
	}
}


//--------------------------------------------------------------------------------------------------------------
void SpriteRenderer::Register( Sprite* newSprite, const char* newLayerName /*= nullptr*/ )
{
	RenderLayer* layer = SpriteRenderer::CreateOrGetLayer( newSprite->GetLayerID(), newLayerName );
		//Background layers could go negative from a default of 0.
		//Going by intervals: 100 as enemy layer, 200 as player layer, 300 as bullet layer, 400 as foreground, -100 as background, -200 as secondary background, and +1000 as UI layer. Lets you add layers in between without shifting all else.
		//Recommends named constants over an enum to not lock it down on the engine side.
	layer->AddSprite( newSprite );
	SpriteRenderer::ResizeSprite( newSprite );
}


//--------------------------------------------------------------------------------------------------------------
void SpriteRenderer::Unregister( Sprite* sprite )
{
	RenderLayer* layer = SpriteRenderer::CreateOrGetLayer( sprite->GetLayerID() );
		//Background layers could go negative from a default of 0.
		//Going by intervals: 100 as enemy layer, 200 as player layer, 300 as bullet layer, 400 as foreground, -100 as background, -200 as secondary background, and +1000 as UI layer. Lets you add layers in between without shifting all else.
		//Recommends named constants over an enum to not lock it down on the engine side.
	layer->RemoveSprite( sprite );
}	


//--------------------------------------------------------------------------------------------------------------
RenderLayer* SpriteRenderer::CreateOrGetLayer( RenderLayerID layerID, const char* newLayerName /*= nullptr*/ )
{
	for ( const SpriteLayerRegistryPair& layerPair : s_spriteLayers )
	{
		if ( layerPair.second->m_layerID == layerID )
			return layerPair.second;
	}

	std::string name;
	name = ( newLayerName == nullptr ) ? "Unnamed" : newLayerName;
	s_spriteLayers.insert( SpriteLayerRegistryPair( layerID, new RenderLayer( layerID, name ) ) );
	return s_spriteLayers.at( layerID );
}


//--------------------------------------------------------------------------------------------------------------
void SpriteRenderer::CopySpriteIntoMesh( Sprite* sprite )
{
	//The IBO and draw instructions will stay the same, but the VBO needs to be overwritten in-place and sent to the GPU again.
	AABB2f worldBounds = sprite->GetVirtualBoundsInWorld();

	//Order taken from CreateQuadMesh used in SpriteRenderer::Startup, to let us keep the same IBO.
	const Vertex2D_PCT vertices[] =
	{
		Vertex2D_PCT( Vector2f( worldBounds.mins.x, worldBounds.maxs.y ),	sprite->GetTint(),	Vector2f( 0.f, 0.f ) ), //Top-left.		//0
		Vertex2D_PCT( Vector2f( worldBounds.maxs.x, worldBounds.mins.y ),	sprite->GetTint(),	Vector2f( 1.f, 1.f ) ), //Bottom-right.	//1
		Vertex2D_PCT( worldBounds.mins,										sprite->GetTint(),	Vector2f( 0.f, 1.f ) ), //Bottom-left.	//2
		Vertex2D_PCT( worldBounds.maxs,										sprite->GetTint(),	Vector2f( 1.f, 0.f ) ) //Top-right.	//3
	}; //Original BL is mins, original TR is maxs. Recall that a UV flip is required to compensate for OGL/stbi importing upside-down on y.

	s_spriteMesh->SetThenUpdateMeshBuffers( 4, (void*)vertices );
}


//--------------------------------------------------------------------------------------------------------------
void SpriteRenderer::AddLayerEffect( RenderLayerID layerID, FramebufferEffect* fboEffect )
{
	RenderLayer* layer = CreateOrGetLayer( layerID );
	layer->AddFboEffect( fboEffect );
}


//--------------------------------------------------------------------------------------------------------------
void SpriteRenderer::RenderSprite( Sprite* sprite, bool isLayerScrolling )
{
	if ( !sprite->IsEnabled() )
		return;

	if ( !sprite->IsVisible() )
		return;

	SpriteRenderer::CopySpriteIntoMesh( sprite );

	Material* spriteMat = sprite->GetMaterial();
	if ( spriteMat == nullptr )
		spriteMat = s_defaultSpriteMaterial;
	
	TODO( "Move the ortho uniform update to not happen per-frame, only as virt size is updated." );
	Matrix4x4f ortho( COLUMN_MAJOR );
	ortho.ClearToOrthogonalProjection( s_virtualScreenSize.x, s_virtualScreenSize.y, -1.f, 1.f, ortho.GetOrdering() );
		// The values of zNear and zFar don't matter as long as former/latter are -/+, using 0 would cause division by 0.
		// Could set the virtual size per layer, to support zooming in and out per layer( i.e.changing - 1 and +1 / zNear and zFar above ).
		// Can set this as a member on TheGame or per layer depending on whether you do per - layer features.

	Matrix4x4f view( COLUMN_MAJOR );
	if ( isLayerScrolling ) //Else it defaults to default ctor's identity matrix, so no scroll.
		view = s_activeCamera->GetViewTransform();

#ifdef PLATFORM_RIFT_CV1
	//Overwrite by adding in offsets based on VR HMD.
	int eye = g_theRenderer->GetRiftContext()->currentEye;
	view = g_theRenderer->CalcRiftViewMatrixMyBasis( eye, s_activeCamera );
	ortho = g_theRenderer->CalcRiftOrthoProjMatrixMyBasis( eye );
#endif

	spriteMat->SetMatrix4x4( "uView", false, &view );
	spriteMat->SetMatrix4x4( "uProj", false, &ortho );

	spriteMat->SetTexture( "uTexDiffuse", sprite->GetDiffuseTextureID() );
	SpriteRenderer::s_spriteMeshRenderer->SetMaterial( spriteMat, true );
	
	SpriteRenderer::s_spriteMeshRenderer->Render();
}


//--------------------------------------------------------------------------------------------------------------
void SpriteRenderer::SetLayerVirtualSize( RenderLayerID layerID, float lengthX, float lengthY )
{
	if ( s_spriteLayers.count( layerID ) == 0 )
	{
		ERROR_RECOVERABLE( "SetLayerVirtualUnitLength called for a non-existent layer!" );
		return;
	}

	RenderLayer* layer = s_spriteLayers.at( layerID );
	layer->UpdateVirtualSize( lengthY, lengthX ); //Very stupid workaround until I can understand where it's mixing x and y.

	//Now update sprites within this layer.
	for ( Sprite* sprite : layer->m_sprites )
		SpriteRenderer::ResizeSprite( sprite );
}


//--------------------------------------------------------------------------------------------------------------
void SpriteRenderer::ResizeSprite( Sprite* sprite )
{
	//Virtual conversion setup.
	Vector2f dimensionsInPixelsAsFloats;
	dimensionsInPixelsAsFloats.x = static_cast<float>( sprite->GetDiffuseTexture()->GetTextureDimensions().x );
	dimensionsInPixelsAsFloats.y = static_cast<float>( sprite->GetDiffuseTexture()->GetTextureDimensions().y );
//	ASSERT_OR_DIE( dimensionsInPixelsAsFloats.x <= importSize && dimensionsInPixelsAsFloats.y <= importSize, "Sprites should not exceed import size!" );
	Vector2f dimsWithCancelledPx = dimensionsInPixelsAsFloats / SpriteRenderer::GetImportSize(); //Because of above assert, should always be 0-1.

	//Actual conversion: use above 0-1 to scale the virtual screen amounts and get the sprite's virtual size on it.
	Vector2f layerVirtualSize = s_spriteLayers[ sprite->GetLayerID() ]->GetLayerVirtualSize();
	if ( layerVirtualSize == RenderLayer::NO_CUSTOM_VIRTUAL_SIZE )
		sprite->SetVirtualSize( dimsWithCancelledPx * SpriteRenderer::GetDefaultVirtualSize() );
	else
		sprite->SetVirtualSize( dimsWithCancelledPx * layerVirtualSize );
}


//--------------------------------------------------------------------------------------------------------------
void SpriteRenderer::Update( float deltaSeconds )
{	
	if ( s_spriteLayers.size() == 0 )
		return;

	for ( const SpriteLayerRegistryPair& layerPair : s_spriteLayers )
	{
		for ( Sprite* sprite : layerPair.second->m_sprites )
			sprite->Update( deltaSeconds ); //Primarily for animations.

		std::vector<ParticleSystem*>& particleSystems = layerPair.second->m_particleSystems;
		for ( size_t index = 0; index < particleSystems.size(); index++ )
		{
			ParticleSystem* system = particleSystems[ index ];

			if ( system->IsExpired() )
			{
				particleSystems[ index ] = particleSystems[ particleSystems.size() - 1 ];

				--index;

				particleSystems.pop_back();
			}

			system->Update( deltaSeconds ); //We don't want it to be expired last, or it could update and immediately die without getting rendered.
		}
	}


	TODO( "Move below into TheRenderer instead of SpriteRenderer--see code review, doesn't belong here." );
	//The camera is GetVirtualScreenBounds() centered on activeCam.m_worldPosition:
	Vector2f halfVirtualScreenSize = GetVirtualScreenDimensions() * 0.5f;
	AABB2f cameraBoundsInWorld;
	cameraBoundsInWorld.mins = s_activeCamera->m_worldPosition - halfVirtualScreenSize;
	cameraBoundsInWorld.maxs = s_activeCamera->m_worldPosition + halfVirtualScreenSize;

	s_numSpritesCulled = 0;
	for ( const SpriteLayerRegistryPair& layerPair : s_spriteLayers )
	{
		if ( !layerPair.second->m_enabled )
			continue;

		//If it's disabled, it's not in a layer, so we don't have to worry about Sprite::IsEnabled().
		for ( Sprite* sprite : layerPair.second->m_sprites )
		{
			//Culling messes up when the view is rotated, because then the post-rotation corners aren't maxs and mins as expected.
			bool visible = DoAABBsOverlap( cameraBoundsInWorld, sprite->GetVirtualBoundsInWorld() );
			if ( !s_shouldCull )
				visible = true;

			if ( visible && !sprite->IsVisible() )
				sprite->Show();
			else if ( !visible && sprite->IsVisible() )
				sprite->Hide();

			if ( !visible )
			{
				++s_numSpritesCulled;
				continue;
			}
		}
	}
}
