#include "Game/TheGame.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Renderer/TheRenderer.hpp"
#include "Engine/Renderer/SpriteRenderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Light.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Camera3D.hpp"
#include "Engine/Math/Camera2D.hpp"

//Game-side Use of SD3 Rendering System Elements
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Renderer/FramebufferEffect.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"

//Game-side Use of AES Rendering System Elements
#include "Engine/Renderer/MeshBuilder.hpp"

#include "Game/Entities/Player.hpp"
#include "Game/Entities/Deflector.hpp"
#include "Game/World.hpp"



//-----------------------------------------------------------------------------
void TheGame::CreateShaderPrograms()
{
	const unsigned int NUM_SHADER_PROGRAMS = 58;
	const unsigned int NUM_FBO_EFFECTS = 27;

	//Need to test and see if this pulls from subdirs.

	TODO( "Use below SD4 method instead to grab shaders--but how to map VS to FS?" );
	// char shaderName[ 80 ];
	// std::vector<std::string> vertexShaderProgramFileNames = EnumerateFilesInDirectory( "Data/Shaders", "*.vert" );
	// for ( const std::string& filename : vertexShaderProgramFileNames )
	// {
	// 	sscanf_s( xmlFilename, "Data/XML/Biomes/%[^.].vert", &shaderName, _countof( shaderName ) );
	// 	//do something with name
	// }
	// std::vector<std::string> fragmentShaderProgramFileNames = EnumerateFilesInDirectory( "Data/Shaders", "*.frag" );
	// for ( const std::string& filename : vertexShaderProgramFileNames )
	// {
	// 	sscanf_s( xmlFilename, "Data/XML/Biomes/%[^.].vert", &shaderName, _countof( shaderName ) );
	// 	//do something with name
	// }

	const char* FBO_VERTEX_SHADER_PATH = "Data/Shaders/PostProcessing/post.vert";

	const char* shaderProgramNames[ NUM_SHADER_PROGRAMS ] = {
		"FixedVertexFormat",
		"TimeFX",
		"BlinnPhong_PCT",

		"OverrideExample",

		"UnityChan_Body",
		"UnityChan_Eyebase",
		"UnityChan_EyeIrisL",
		"UnityChan_EyeIrisR",
		"UnityChan_Eyeline",
		"UnityChan_Face",
		"UnityChan_Hair",
		"UnityChan_Skin",
		"UnityChan_Body",
		"UnityChan_Cheek",
		"BlinnPhongTBN_PCUTB",
		"BlinnPhongTBN_Superset",
		"Triplanar",
		"Parallax",
		"Dissolve",
		"Projector",
		"Skinweights_Superset",
		"SkinDebug_BoneWeights",
		"SkinDebug_BoneIndices",
		"PassthroughColor",
		"PassthroughUV0",
		"PassthroughUV1",
		"PassthroughUV2",
		"PassthroughUV3",
		"PassthroughTangent",
		"PassthroughBitangent",
		"PassthroughNormal",
		"Mandelbrot",

		"PostProcessGrayscale",
		"PostProcessCMYK_C",
		"PostProcessCMYK_M",
		"PostProcessCMYK_Y",
		"PostProcessCMYK_K",
		"PostProcessHueShift",
		"PostProcessBunnyBlur",
		"PostProcessGaussianBlur",
		"PostProcessChromakey",
		"PostProcessAnaglyph",
		"PostProcessEdgeDetectByLuminance",
		"PostProcessEmboss",
		"PostProcessToonShader",
		"PostProcessZoom",
		"PostProcessTwirl",

		"PostProcessNegative",
		"PostProcessBrightness",
		"PostProcessContrast",
		"PostProcessSaturation",
		"PostProcessSharpness",

		"PostProcessColorMask",
		"PostProcessDepthOnly",
		"PostProcessEdgeDetectByDepthBuffer",
		"PostProcessEdgeDetectQuantized",
		"PostProcessObama",
		"PostProcessRipple"
	};
	const char* vertexShaderPaths[ NUM_SHADER_PROGRAMS ] = {
		"Data/Shaders/fvf.vert",
		"Data/Shaders/timeFx.vert",
		"Data/Shaders/blinnPhong.vert",

		"Data/Shaders/Sprites/overrideExample.vert",

		"Data/Shaders/Animation/skinweights.vert",
		"Data/Shaders/Animation/skinweights.vert",
		"Data/Shaders/Animation/skinweights.vert",
		"Data/Shaders/Animation/skinweights.vert",
		"Data/Shaders/Animation/skinweights.vert",
		"Data/Shaders/Animation/skinweights.vert",
		"Data/Shaders/Animation/skinweights.vert",
		"Data/Shaders/Animation/skinweights.vert",
		"Data/Shaders/Animation/skinweights.vert",
		"Data/Shaders/Animation/skinweights.vert",

		"Data/Shaders/blinnPhong.vert",
		"Data/Shaders/blinnPhong.vert",

		"Data/Shaders/triplanar.vert",
		"Data/Shaders/blinnPhong.vert",
		"Data/Shaders/blinnPhong.vert",
		"Data/Shaders/blinnPhong.vert",

		"Data/Shaders/Animation/skinweights.vert",
		"Data/Shaders/Animation/skinDebug_boneWeights.vert",
		"Data/Shaders/Animation/skinDebug_boneIndices.vert",

		"Data/Shaders/passthrough.vert",
		"Data/Shaders/passthrough.vert",
		"Data/Shaders/passthrough.vert",
		"Data/Shaders/passthrough.vert",
		"Data/Shaders/passthrough.vert",
		"Data/Shaders/passthrough.vert",
		"Data/Shaders/passthrough.vert",
		"Data/Shaders/passthrough.vert",
		"Data/Shaders/passthrough.vert" //and after this we use the above FBO vert shader
	};
	const char* fragmentShaderPaths[ NUM_SHADER_PROGRAMS ] = {
		"Data/Shaders/fvf.frag",
		"Data/Shaders/timeFx.frag",
		"Data/Shaders/blinnPhongMultipleLights.frag",

		"Data/Shaders/Sprites/overrideExample.frag",

		"Data/Shaders/blinnPhongMultipleLightsUnityChan_Body.frag",
		"Data/Shaders/blinnPhongMultipleLightsUnityChan_Eyebase.frag",
		"Data/Shaders/blinnPhongMultipleLightsUnityChan_EyeIrisL.frag",
		"Data/Shaders/blinnPhongMultipleLightsUnityChan_EyeIrisR.frag",
		"Data/Shaders/blinnPhongMultipleLightsUnityChan_Eyeline.frag",
		"Data/Shaders/blinnPhongMultipleLightsUnityChan_Face.frag",
		"Data/Shaders/blinnPhongMultipleLightsUnityChan_Hair.frag",
		"Data/Shaders/blinnPhongMultipleLightsUnityChan_Skin.frag",
		"Data/Shaders/blinnPhongMultipleLightsUnityChan_Body.frag",
		"Data/Shaders/blinnPhongMultipleLightsUnityChan_Cheek.frag",

		"Data/Shaders/blinnPhongMultipleLights.frag",
		"Data/Shaders/blinnPhongMultipleLights.frag",

		"Data/Shaders/triplanar.frag",
		"Data/Shaders/parallax.frag",
		"Data/Shaders/dissolve.frag",
		"Data/Shaders/projector.frag",

		"Data/Shaders/passthroughColor.frag",
		"Data/Shaders/passthroughColor.frag",
		"Data/Shaders/passthroughColor.frag",

		"Data/Shaders/passthroughColor.frag",
		"Data/Shaders/passthroughUV0.frag",
		"Data/Shaders/passthroughUV1.frag",
		"Data/Shaders/passthroughUV2.frag",
		"Data/Shaders/passthroughUV3.frag",
		"Data/Shaders/passthroughTangent.frag",
		"Data/Shaders/passthroughBitangent.frag",
		"Data/Shaders/passthroughNormal.frag",
		"Data/Shaders/mandelbrot.frag",

		"Data/Shaders/PostProcessing/postGrayscale.frag",
		"Data/Shaders/PostProcessing/postCMYK_C.frag",
		"Data/Shaders/PostProcessing/postCMYK_M.frag",
		"Data/Shaders/PostProcessing/postCMYK_Y.frag",
		"Data/Shaders/PostProcessing/postCMYK_K.frag",
		"Data/Shaders/PostProcessing/postHueShift.frag",
		"Data/Shaders/PostProcessing/postBunnyBlur.frag",
		"Data/Shaders/PostProcessing/postGaussianBlur.frag",
		"Data/Shaders/PostProcessing/postChromakey.frag",
		"Data/Shaders/PostProcessing/postAnaglyph.frag",
		"Data/Shaders/PostProcessing/postEdgeDetectionByLuminance.frag",
		"Data/Shaders/PostProcessing/postEmboss.frag",
		"Data/Shaders/PostProcessing/postToonShader.frag",
		"Data/Shaders/PostProcessing/postZoom.frag",
		"Data/Shaders/PostProcessing/postTwirl.frag",

		"Data/Shaders/PostProcessing/postNegative.frag",
		"Data/Shaders/PostProcessing/postBrightness.frag",
		"Data/Shaders/PostProcessing/postContrast.frag",
		"Data/Shaders/PostProcessing/postSaturation.frag",
		"Data/Shaders/PostProcessing/postSharpness.frag",

		"Data/Shaders/PostProcessing/postColorMask.frag",
		"Data/Shaders/PostProcessing/postDepthOnly.frag",
		"Data/Shaders/PostProcessing/postEdgeDetectionByDepthBuffer.frag",
		"Data/Shaders/PostProcessing/postEdgeDetectionByDepthBufferWithQuantization.frag",
		"Data/Shaders/PostProcessing/postObama.frag",
		"Data/Shaders/PostProcessing/postRipple.frag"
	};
	const VertexDefinition* shaderProgramVertexDefinitions[ NUM_SHADER_PROGRAMS ] = {
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,

		&Vertex2D_PCT::DEFINITION,

		&Vertex3D_PCTWI::DEFINITION, //indices #3-12 = Unity-chan
		&Vertex3D_PCTWI::DEFINITION,
		&Vertex3D_PCTWI::DEFINITION,
		&Vertex3D_PCTWI::DEFINITION,
		&Vertex3D_PCTWI::DEFINITION,
		&Vertex3D_Superset::DEFINITION,
		&Vertex3D_PCTWI::DEFINITION,
		&Vertex3D_PCTWI::DEFINITION,
		&Vertex3D_PCTWI::DEFINITION,
		&Vertex3D_PCTWI::DEFINITION,

		&Vertex3D_PCUTB::DEFINITION, //#13 - BlinnPhong_PCUTB

		&Vertex3D_Superset::DEFINITION, //#14 - BlinnPhong_Superset

		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,

		&Vertex3D_Superset::DEFINITION, //#19 - Skinweights_Superset
		&Vertex3D_Superset::DEFINITION, //#20-26 - Debug Pass-through / SetRenderModes
		&Vertex3D_Superset::DEFINITION,
		&Vertex3D_Superset::DEFINITION,
		&Vertex3D_Superset::DEFINITION,
		&Vertex3D_Superset::DEFINITION,
		&Vertex3D_Superset::DEFINITION,
		&Vertex3D_Superset::DEFINITION,
		&Vertex3D_Superset::DEFINITION,
		&Vertex3D_Superset::DEFINITION,

		&Vertex3D_PCUTB::DEFINITION, //indices #27-#NUM_SHADER_PROGRAMS = FBOs
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
		&Vertex3D_PCUTB::DEFINITION,
	};

	TODO( "Fragment Shader + Noise Function" );

	for ( unsigned int shaderProgramIndex = 0; shaderProgramIndex < NUM_SHADER_PROGRAMS; shaderProgramIndex++ )
		ShaderProgram::CreateOrGetShaderProgram(
			shaderProgramNames[ shaderProgramIndex ],
			( shaderProgramIndex >( NUM_SHADER_PROGRAMS - NUM_FBO_EFFECTS ) ) ? FBO_VERTEX_SHADER_PATH : vertexShaderPaths[ shaderProgramIndex ],
			fragmentShaderPaths[ shaderProgramIndex ],
			shaderProgramVertexDefinitions[ shaderProgramIndex ] );
}
void TheGame::CreateUniformValues()
{
	Texture* texture = Texture::CreateOrGetTexture( "Data/Images/Paused.png" );
	//	Texture* diffuseAlienTex = Texture::CreateOrGetTexture( "Data/Images/alienfloor1_diffuse.png" );
	//	Texture* normalAlienTex = Texture::CreateOrGetTexture( "Data/Images/alienfloor1_normal.png" );
	//	Texture* specularAlienTex = Texture::CreateOrGetTexture( "Data/Images/alienfloor1_specular.png" );
	Texture* diffuseStoneTex = Texture::CreateOrGetTexture( "Data/Images/stone_diffuse.png" );
	Texture* normalStoneTex = Texture::CreateOrGetTexture( "Data/Images/stone_normal.png" );
	Texture* specularStoneTex = Texture::CreateOrGetTexture( "Data/Images/stone_spec.png" );
	Texture* emissiveStoneTex = Texture::CreateOrGetTexture( "Data/Images/stone_emissive.png" );
	Texture* ambientOcclusionStoneTex = Texture::CreateOrGetTexture( "Data/Images/stone_ao.png" );
	Texture* heightsStoneTex = Texture::CreateOrGetTexture( "Data/Images/stone_heights.png" );
	Texture* dissolveTex = Texture::CreateOrGetTexture( "Data/Images/dissolve.png" );
	Texture* projectedTex = Texture::CreateOrGetTexture( "Data/Images/projected.png" );
	Texture* diffuseStoneTexXY = Texture::CreateOrGetTexture( "Data/Images/Triplanar/stone_diffuseXY.png" );
	Texture* diffuseStoneTexXZ = Texture::CreateOrGetTexture( "Data/Images/Triplanar/stone_diffuseXZ.png" );
	Texture* diffuseStoneTexYZ = Texture::CreateOrGetTexture( "Data/Images/Triplanar/stone_diffuseYZ.png" );
	Texture* specularStoneTexXY = Texture::CreateOrGetTexture( "Data/Images/Triplanar/stone_specXY.png" );
	Texture* specularStoneTexXZ = Texture::CreateOrGetTexture( "Data/Images/Triplanar/stone_specXZ.png" );
	Texture* specularStoneTexYZ = Texture::CreateOrGetTexture( "Data/Images/Triplanar/stone_specYZ.png" );
	Texture* normalStoneTexXY = Texture::CreateOrGetTexture( "Data/Images/Triplanar/stone_normalXY.png" );
	Texture* normalStoneTexXZ = Texture::CreateOrGetTexture( "Data/Images/Triplanar/stone_normalXZ.png" );
	Texture* normalStoneTexYZ = Texture::CreateOrGetTexture( "Data/Images/Triplanar/stone_normalYZ.png" );

	Texture* bodyDiffuseTex = Texture::CreateOrGetTexture( "Data/Images/Vocaloid/body_01.tga" );
	Texture* cheekDiffuseTex = Texture::CreateOrGetTexture( "Data/Images/Vocaloid/cheek_00.tga" );
	Texture* eyeIrisDiffuseTexL = Texture::CreateOrGetTexture( "Data/Images/Vocaloid/eye_iris_L_00.tga" );
	Texture* eyeIrisDiffuseTexR = Texture::CreateOrGetTexture( "Data/Images/Vocaloid/eye_iris_R_00.tga" );
	Texture* eyelineDiffuseTex = Texture::CreateOrGetTexture( "Data/Images/Vocaloid/eyeline_00.tga" );
	Texture* faceDiffuseTex = Texture::CreateOrGetTexture( "Data/Images/Vocaloid/face_00.tga" );
	Texture* hairDiffuseTex = Texture::CreateOrGetTexture( "Data/Images/Vocaloid/hair_01.tga" );
	Texture* skinDiffuseTex = Texture::CreateOrGetTexture( "Data/Images/Vocaloid/skin_01.tga" );

	Matrix4x4f projectorViewMatrix;
	projectorViewMatrix.ClearToRotationMatrix_MyBasis( 0.f, 90.f, 0.f, COLUMN_MAJOR );

	static Sampler noRepeat;

	//Won't take literals, have to make initial temps.
	float lightIntensity = .25f;
	float specularExponent = 4.f;
	float specularIntensity = 4.f;
	float fogMinDist = 4.f;
	float fogMaxDist = 16.f;
	float timerDurationSeconds = 1.f;

	for ( auto& sp : *ShaderProgram::GetRegistry() )
	{
		sp.second->SetMatrix4x4( "uProjectorView", false, &projectorViewMatrix );

		sp.second->SetColor( "uColor", &Rgba::WHITE );
		sp.second->SetColor( "uFogColor", &Rgba::DARK_GRAY ); //Be sure it matches the screen clear color, else will fade to different color!
		sp.second->SetColor( "uAmbientLight", &Rgba::GRAY );
		sp.second->SetColor( "uLightColor", &Rgba::WHITE );

		sp.second->SetFloat( "uWrappingTimerDuration", &timerDurationSeconds );
		sp.second->SetFloat( "uLightIntensity", &lightIntensity );
		sp.second->SetFloat( "uSpecularExponent", &specularExponent );
		sp.second->SetFloat( "uSpecularIntensity", &specularIntensity );
		sp.second->SetFloat( "uFogMinDist", &fogMinDist );
		sp.second->SetFloat( "uFogMaxDist", &fogMaxDist );

		sp.second->SetTexture( "uTexDiffuseXY", diffuseStoneTexXY->GetTextureID() );
		sp.second->SetTexture( "uTexDiffuseXZ", diffuseStoneTexXZ->GetTextureID() );
		sp.second->SetTexture( "uTexDiffuseYZ", diffuseStoneTexYZ->GetTextureID() );
		sp.second->SetTexture( "uTexNormalXY", normalStoneTexXY->GetTextureID() );
		sp.second->SetTexture( "uTexNormalXZ", normalStoneTexXZ->GetTextureID() );
		sp.second->SetTexture( "uTexNormalYZ", normalStoneTexYZ->GetTextureID() );
		sp.second->SetTexture( "uTexSpecularXY", specularStoneTexXY->GetTextureID() );
		sp.second->SetTexture( "uTexSpecularXZ", specularStoneTexXZ->GetTextureID() );
		sp.second->SetTexture( "uTexSpecularYZ", specularStoneTexYZ->GetTextureID() );

		sp.second->SetTexture( "uDiffuseTex", texture->GetTextureID() );
		sp.second->SetTexture( "uTexDiffuse", diffuseStoneTex->GetTextureID() );

		sp.second->SetTexture( "uTexDiffuseBody", bodyDiffuseTex->GetTextureID() );
		sp.second->SetTexture( "uTexDiffuseCheek", cheekDiffuseTex->GetTextureID() );
		sp.second->SetTexture( "uTexDiffuseEyeIrisL", eyeIrisDiffuseTexL->GetTextureID() );
		sp.second->SetTexture( "uTexDiffuseEyeIrisR", eyeIrisDiffuseTexR->GetTextureID() );
		sp.second->SetTexture( "uTexDiffuseEyebase", projectedTex->GetTextureID() );
		sp.second->SetTexture( "uTexDiffuseEyeline", eyelineDiffuseTex->GetTextureID() );
		sp.second->SetTexture( "uTexDiffuseFace", faceDiffuseTex->GetTextureID() );
		sp.second->SetTexture( "uTexDiffuseHair", hairDiffuseTex->GetTextureID() );
		sp.second->SetTexture( "uTexDiffuseSkin", skinDiffuseTex->GetTextureID() );

		sp.second->SetTexture( "uTexNormal", normalStoneTex->GetTextureID() );
		sp.second->SetTexture( "uTexSpecular", specularStoneTex->GetTextureID() );
		sp.second->SetTexture( "uTexDiffuse", diffuseStoneTex->GetTextureID() );
		sp.second->SetTexture( "uTexNormal", normalStoneTex->GetTextureID() );
		sp.second->SetTexture( "uTexSpecular", specularStoneTex->GetTextureID() );
		sp.second->SetTexture( "uTexEmissive", emissiveStoneTex->GetTextureID() );
		sp.second->SetTexture( "uTexAmbientOcclusion", ambientOcclusionStoneTex->GetTextureID() );
		sp.second->SetTexture( "uTexHeights", heightsStoneTex->GetTextureID() );
		sp.second->SetTexture( "uTexDissolve", dissolveTex->GetTextureID() );
		sp.second->SetTexture( "uTexProjected", projectedTex->GetTextureID() );

		sp.second->SetSampler( "uDiffuseTex", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexDiffuse", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexNormal", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexSpecular", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexEmissive", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexAmbientOcclusion", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexHeights", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexDissolve", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexProjected", noRepeat.GetSamplerID() );
		sp.second->SetSampler( "uTexDiffuseXY", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexDiffuseXZ", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexDiffuseYZ", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexNormalXY", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexNormalXZ", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexNormalYZ", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexSpecularXY", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexSpecularXZ", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexSpecularYZ", TheRenderer::DEFAULT_SAMPLER_ID );

		sp.second->SetSampler( "uTexDiffuseBody", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexDiffuseCheek", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexDiffuseEyeIrisL", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexDiffuseEyeIrisR", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexDiffuseEyebase", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexDiffuseEyeline", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexDiffuseFace", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexDiffuseHair", TheRenderer::DEFAULT_SAMPLER_ID );
		sp.second->SetSampler( "uTexDiffuseSkin", TheRenderer::DEFAULT_SAMPLER_ID );
	}
}
void TheGame::CreateMeshRenderers()
{
	AABB3f cubeBounds;
	cubeBounds.mins = Vector3f( -1.f, -1.f, -1.f );
	cubeBounds.maxs = Vector3f( 1.f, 1.f, 1.f );
	g_theRenderer->CreateCubeRenderer3D_PCUTB( cubeBounds, "BlinnPhongTBN_PCUTB", TheRenderer::DEFAULT_RENDER_STATE_3D );
}
void TheGame::CreateFBOs()
{
	const char* fboEffectShaderProgramNames[ 26 ] = {
#pragma region Game Specific MeshRenderer Setup
		"PostProcessGrayscale",
		"PostProcessCMYK_C",
		"PostProcessCMYK_M",
		"PostProcessCMYK_Y",
		"PostProcessCMYK_K",
		"PostProcessHueShift",
		"PostProcessBunnyBlur",
		"PostProcessGaussianBlur",
		"PostProcessChromakey",
		"PostProcessAnaglyph",
		"PostProcessEdgeDetectByLuminance",
		"PostProcessEmboss",
		"PostProcessToonShader",
		"PostProcessZoom",
		"PostProcessTwirl",
		"PostProcessNegative",
		"PostProcessBrightness",
		"PostProcessContrast",
		"PostProcessSaturation",
		"PostProcessSharpness",
		"PostProcessColorMask",
		"PostProcessDepthOnly",
		"PostProcessEdgeDetectByDepthBuffer",
		"PostProcessEdgeDetectQuantized",
		"PostProcessObama",
		"PostProcessRipple"
#pragma endregion
	};
	g_theRenderer->AddFullscreenFboEffects( fboEffectShaderProgramNames, 26 );

#pragma region Game Specific Uniform Setup
	//Configure (each?) FBO's render target(s) as the shader program's input uniforms.
	Vector2f anaglyphOffset = Vector2f( .005f, 0.f );
	float quantizeLevel = 3.f;
	Texture* chromakeyBackgroundTex = Texture::CreateOrGetTexture( "Data/Images/dissolve.png" );
	for ( const FboEffectRegistryPair& fboEffectPair : *FramebufferEffect::GetFboEffectRendererRegistry() )
	{
		FramebufferEffect* fboEffect = fboEffectPair.second;
		MeshRenderer* mr = fboEffect->m_fboEffectRenderer;

		mr->SetSampler( "uTexDiffuse", TheRenderer::DEFAULT_SAMPLER_ID );
		mr->SetTexture( "uTexDiffuseChromakeyBG", chromakeyBackgroundTex->GetTextureID() );
		mr->SetSampler( "uTexDiffuseChromakeyBG", TheRenderer::DEFAULT_SAMPLER_ID );
		mr->SetSampler( "uTexDepth", TheRenderer::DEFAULT_SAMPLER_ID );
		mr->SetColor( "uColorMask", &Rgba::RED );
		mr->SetColor( "uColor", &Rgba::WHITE );
		mr->SetVector2( "uOffset", &anaglyphOffset );
		mr->SetFloat( "uQuantizationLevel", &quantizeLevel );

		//If you output more things to FBOs like an out normal map, add them here, e.g. for deferred rendering.

		//Deferred rendering: you write shaders outputting TBN, color, depth as five "g-buffers".

		//Then all lighting is calculated per-pixel using those 5 g[raphics]-buffers.
	}
#pragma endregion
}


//-----------------------------------------------------------------------------
void TheGame::CreateSceneLights() //Ensure that MAX_LIGHTS is set to the right value first!
{
	float scale = 1.f; //float sqrt2 = sqrt( 2.f );

	Light::CreateLight( DIRECTIONAL_LIGHT, Vector3f( scale * 2.f, 0.f, 0.f ), WORLD3D_DOWN + WORLD3D_FORWARD, Rgba( 0.0f, 1.0f, 1.0f ) ); //the 1 global direct light.
	Light::CreateLight( POINT_LIGHT, Vector3f( scale * 1.f, 0.f, 0.f ), WORLD3D_LEFT, Rgba( 1.0f, 1.0f, 1.0f ) ); //the 1 local point light.

	//Create other engine lights. Probably move this engine-side later/wrap into Light::InitLights.
	unsigned int lightsUncreated = LIGHTS_IN_ENGINE_MAX - Light::GetRegistry()->size();
	for ( unsigned int lightIndex = 0; lightIndex < lightsUncreated; lightIndex++ )
		Light::CreateLight( POINT_LIGHT, Vector3f::ZERO, Vector3f::ZERO, Rgba::BLACK ); //All-zero light color prevents contribution to scene.
}


//-----------------------------------------------------------------------------
#pragma region Rendering Functions
void TheGame::Render3D() //Look below in RenderDebug3D for DebugRenderCommand use.
{
	//Render 3D world/map/scene or player representation.
	for ( const MeshRendererRegistryPair& mr : *MeshRenderer::GetMeshRendererRegistry() )
		mr.second->Render();

	for ( Light* light : *Light::GetRegistry() )
		light->Render();

	ShaderProgram::UnbindAnyPrograms();

	//Render any 3D overlays.
}


//-----------------------------------------------------------------------------
void TheGame::RenderDebug3D()
{
	//DebugRenderCommands from Engine, etc. Confer SD3 project code.
}


//-----------------------------------------------------------------------------
bool TheGame::Render2D()
{
	SpriteRenderer::RenderFrame();

	bool didRender = false;
	switch ( GetGameState() )
	{
		case GAME_STATE_MAIN_MENU: didRender = RenderMainMenu2D(); break;
		case GAME_STATE_PLAYING: didRender = RenderPlaying2D(); break;
		case GAME_STATE_PAUSED: didRender = RenderPaused2D(); break;
		case GAME_STATE_GAMEOVER: didRender = RenderGameOver2D(); break;
	}
	return didRender;
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::RenderMainMenu2D()
{
	g_theRenderer->DrawTextProportional2D
		(
			WorldCoords2D( 100.f, 400.f ),
			"P - Play Game",
			.50f
		);
	
	g_theRenderer->DrawTextProportional2D
		(
			WorldCoords2D( 100.f, 300.f ),
			"WASD - Move",
			.50f,
			nullptr,
			Rgba::GRAY
		);

	g_theRenderer->DrawTextProportional2D
		(
			WorldCoords2D( 100.f, 200.f ),
			"Enter - Gain Terrain's Deflector",
			.50f,
			nullptr,
			Rgba::GRAY
		);

	g_theRenderer->DrawTextProportional2D
		(
			WorldCoords2D( 100.f, 100.f ),
			"Spacebar - Launch Deflector",
			.50f,
			nullptr,
			Rgba::GRAY
		);

	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::RenderPlaying2D()
{
	g_theRenderer->DrawTextProportional2D
		(
			Vector2f( 100.f, (float)g_theRenderer->GetScreenHeight() - 50.f ),
			Stringf( "%d Enemies / %d Waves Remaining", m_numWaveEnemiesLeft, m_arenaCycle[ m_currentArena ]->m_waves.size() - m_arenaCycle[ m_currentArena ]->m_currentWave ),
			.30f
		);

	g_theRenderer->DrawTextProportional2D
		(
			Vector2f( 100.f, (float)g_theRenderer->GetScreenHeight() - 150.f ),
			Stringf( "%s Deflector: %s", GetCurrentTerrain().c_str(), Deflector::GetDeflectorDescForTerrain( GetCurrentTerrain() ).c_str() ),
			.30f
			);

	byte_t newAlpha = static_cast<byte_t>( 255 * RangeMap( m_fadeoutTimer, m_FADEOUT_LENGTH_SECONDS, 0.f, 0.f, 1.f ) );
	std::string meterStr = Stringf( "AuraMeter: %d / %d", m_player->GetAurameterValue(), MAX_AURAMETER );
	g_theRenderer->DrawTextProportional2D
		(
			Vector2f( (float)g_theRenderer->GetScreenCenter().x - ( g_theRenderer->CalcTextPxWidthUpToIndex( meterStr, meterStr.size() ) * .5f ), 50.f ),
			meterStr,
			.25f,
			nullptr,
			( m_player->GetAurameterValue() >= MAX_AURAMETER ) ? Rgba( 255, 255, 255, newAlpha ) : Rgba::GRAY
		);

	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::RenderPaused2D()
{
	Vector2f center = g_theRenderer->GetScreenCenter();

	g_theRenderer->DrawTextProportional2D
		(
			WorldCoords2D( 250.f, center.y + 150.f ),
			"Press (P)ause Again to Unpause",
			.50f
		);
	g_theRenderer->DrawTextProportional2D
		(
			WorldCoords2D( 250.f, center.y + 50.f ),
			"or (R)eturn to Exit to Main Menu",
			.50f
		);

	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::RenderGameOver2D()
{	
	static Rgba topLeftOverlayColor = Rgba::GRAY + Rgba::HALF_OPAQUE_BLACK;
	static Rgba topRightOverlayColor = Rgba::DARK_GRAY + Rgba::HALF_OPAQUE_BLACK;
	static Rgba bottomLeftOverlayColor = Rgba::DARK_GRAY + Rgba::HALF_OPAQUE_BLACK;
	static Rgba bottomRightOverlayColor = Rgba::BLACK + Rgba::HALF_OPAQUE_BLACK;

	byte_t newAlpha = static_cast<byte_t>( 255 * RangeMap( m_fadeoutTimer, m_FADEOUT_LENGTH_SECONDS, 0.f, 0.f, 1.f ) );
	topLeftOverlayColor.alphaOpacity = newAlpha;
	topRightOverlayColor.alphaOpacity = newAlpha;
	bottomLeftOverlayColor.alphaOpacity = newAlpha;
	bottomRightOverlayColor.alphaOpacity = newAlpha;

	g_theRenderer->DrawShadedAABB( VertexGroupingRule::AS_TRIANGLES, AABB2f( Vector2f::ZERO, SpriteRenderer::GetVirtualScreenDimensions() ),
									topLeftOverlayColor, topRightOverlayColor, bottomLeftOverlayColor, bottomRightOverlayColor );

	g_theRenderer->DrawTextMonospaced2D
	( 
		g_theRenderer->GetScreenCenter() - 150.f,
		"Game Over",
		96.f,
		Rgba( 255, 0, 0, newAlpha ),
		nullptr,
		1.f,
		false
	);
	
	g_theRenderer->DrawTextProportional2D
	(
		WorldCoords2D( 150.f, g_theRenderer->GetScreenCenter().y + 50.f ),
		"(P)lay Again, or (R)eturn to Main Menu?",
		.50f
	);

	return true;
}


//-----------------------------------------------------------------------------
void TheGame::RenderDebug2D()
{
	RenderLeftDebugText2D();
	RenderRightDebugText2D();
}


//-----------------------------------------------------------------------------
void TheGame::RenderRightDebugText2D()
{
	g_theRenderer->DrawTextMonospaced2D
		(
			Vector2f( (float)g_theRenderer->GetScreenWidth() - 350.f, (float)g_theRenderer->GetScreenHeight() - 150.f ),
			Stringf( "GameState: %s", GetGameStateName( GetGameState() ) ),
			18.f,
			Rgba::GREEN,
			nullptr,
			.65f
		);
// 	g_theRenderer->DrawTextProportional2D
// 		(
// 			Vector2f( (float)g_theRenderer->GetScreenWidth() - 350.f, (float)g_theRenderer->GetScreenHeight() - 50.f ),
// 			Stringf( "# Sprites Culled: %u", SpriteRenderer::GetNumSpritesCulled() ),
// 			.25f
// 			);
	g_theRenderer->DrawTextProportional2D
		(
			Vector2f( (float)g_theRenderer->GetScreenWidth() - 350.f, (float)g_theRenderer->GetScreenHeight() - 50.f ),
			Stringf( "# Live Particles: %u", SpriteRenderer::GetNumLiveParticles() ),
			.25f
		);
}


//-----------------------------------------------------------------------------
float g_shadowAlphaCounter = 0.f;
void TheGame::RenderLeftDebugText2D()
{
	std::string camPosString;
	std::string camOriString;
	std::string camDirString;

	switch ( s_activeCameraMode )
	{
	case CAMERA_MODE_3D:
		{
		WorldCoords3D camPos = s_playerCamera3D->m_worldPosition;
		EulerAngles camOri = s_playerCamera3D->m_orientation;
		Vector3f camDir = s_playerCamera3D->GetForwardXYZ();

		camPosString = Stringf( "Camera Position: %f %f %f", camPos.x, camPos.y, camPos.z );
		camOriString = Stringf( "Camera Orientation: roll(%f) pitch(%f) yaw(%f)", camOri.m_rollDegrees, camOri.m_pitchDegrees, camOri.m_yawDegrees );
		camDirString = Stringf( "Camera Forward XYZ: %f %f %f", camDir.x, camDir.y, camDir.z );
		break;
		}
	case CAMERA_MODE_2D:
		{
		WorldCoords2D camPos = s_playerCamera2D->m_worldPosition;
		float camOri = s_playerCamera2D->m_orientationDegrees;
		Vector2f camDir = s_playerCamera2D->GetForwardXY();

		camPosString = Stringf( "Camera Position: %f %f", camPos.x, camPos.y );
		camOriString = Stringf( "Camera Orientation: %f (Degrees)", camOri );
		camDirString = Stringf( "Camera Forward XYZ: %f %f", camDir.x, camDir.y );
		break;
		}
	}

	g_theRenderer->DrawTextProportional2D
		(
			Vector2f( 100.f, (float)g_theRenderer->GetScreenHeight() - 50.f ),
			camPosString,
			.25f
		);
	WrapNumberWithinCircularRange( g_shadowAlphaCounter, 0.f, 1.f );
	g_theRenderer->DrawTextProportional2D
		(
			Vector2f( 100.f, (float)g_theRenderer->GetScreenHeight() - 100.f ),
			camOriString,
			.25f,
			nullptr,
			Rgba( .7f, .2f, .7f )
		);
	g_theRenderer->DrawTextProportional2D
		(
			Vector2f( 100.f, (float)g_theRenderer->GetScreenHeight() - 150.f ),
			camDirString,
			.25f,
			nullptr,
			Rgba( .7f, .7f, .7f )
		);

	if ( s_activeCameraMode == CAMERA_MODE_2D )
	{
		g_theRenderer->DrawTextProportional2D
			(
				Vector2f( 100.f, (float)g_theRenderer->GetScreenHeight() - 200.f ),
				Stringf( "Camera Speed Scale: %f", s_playerCamera2D->m_speedScale ),
				.25f,
				nullptr,
				Rgba( .7f, .7f, .7f )
			);
	}

	if ( m_player == nullptr )
		return;

	g_theRenderer->DrawTextProportional2D
		(
			Vector2f( 100.f, (float)g_theRenderer->GetScreenHeight() - 250.f ),
			Stringf( "Player Position: %f %f", m_player->GetPosition().x, m_player->GetPosition().y ),
			.25f
		);
}
#pragma endregion


//-----------------------------------------------------------------------------
void TheGame::InitSceneRendering()
{
	CreateShaderPrograms();

	CreateUniformValues();

	Material::CreateOrGetMaterial( "PassthroughColor", &TheRenderer::DEFAULT_RENDER_STATE_3D, &Vertex3D_Superset::DEFINITION, "PassthroughColor" );
	Material::CreateOrGetMaterial( "PassthroughUV0", &TheRenderer::DEFAULT_RENDER_STATE_3D, &Vertex3D_Superset::DEFINITION, "PassthroughUV0" );
	Material::CreateOrGetMaterial( "PassthroughUV1", &TheRenderer::DEFAULT_RENDER_STATE_3D, &Vertex3D_Superset::DEFINITION, "PassthroughUV1" );
	Material::CreateOrGetMaterial( "PassthroughUV2", &TheRenderer::DEFAULT_RENDER_STATE_3D, &Vertex3D_Superset::DEFINITION, "PassthroughUV2" );
	Material::CreateOrGetMaterial( "PassthroughUV3", &TheRenderer::DEFAULT_RENDER_STATE_3D, &Vertex3D_Superset::DEFINITION, "PassthroughUV3" );
	Material::CreateOrGetMaterial( "PassthroughTangent", &TheRenderer::DEFAULT_RENDER_STATE_3D, &Vertex3D_Superset::DEFINITION, "PassthroughTangent" );
	Material::CreateOrGetMaterial( "PassthroughBitangent", &TheRenderer::DEFAULT_RENDER_STATE_3D, &Vertex3D_Superset::DEFINITION, "PassthroughBitangent" );
	Material::CreateOrGetMaterial( "PassthroughNormal", &TheRenderer::DEFAULT_RENDER_STATE_3D, &Vertex3D_Superset::DEFINITION, "PassthroughNormal" );

	CreateMeshRenderers();

	CreateFBOs();

	CreateSceneLights();
}