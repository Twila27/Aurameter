#pragma once

#include <string>
#include <map>
#include "Engine/Memory/UntrackedAllocator.hpp"
#include "Engine/Renderer/VertexDefinition.hpp"
#include "Engine/Renderer/RenderState.hpp"
#include "Engine/EngineCommon.hpp"


//--------------------------------------------------------------------------------------------------------------
class Shader;
class ShaderProgram;
class Mesh;
class Material;
struct Rgba;
class Sampler;
class Texture;


//--------------------------------------------------------------------------------------------------------------
/*
struct MaterialPass
{
MaterialPass() {};
MaterialPass( ShaderProgram* shaderProgram, RenderState* renderState ) : m_shaderProgram( shaderProgram ), m_renderState( renderState ) {}

ShaderProgram* m_shaderProgram;
RenderState* m_renderState;
}
*/

//--------------------------------------------------------------------------------------------------------------
typedef std::pair< std::string, Material* > MaterialRegistryPair;
typedef std::map< std::string, Material*, std::less<std::string>, UntrackedAllocator<MaterialRegistryPair> > MaterialRegistryMap;


//--------------------------------------------------------------------------------------------------------------
class Material //Has the uniforms, not VBO/IBOs. The "How" to render without the "What" (mesh) to render.
{
public:

	static Material* CreateOrGetMaterial( const std::string& materialName, const RenderState* renderState = nullptr, const VertexDefinition* vertexDefinition = nullptr,
									 const char* shaderProgramName = nullptr, const char* vertexShaderFilePath = nullptr, const char* fragmentShaderFilePath = nullptr );
	static Material* Material::CreateOrGetMaterial( const std::string& materialName, const RenderState* renderState, const VertexDefinition* vertexDefinition,
														  const char* shaderProgramName, Shader* vertexShader, Shader* fragmentShader );
	static bool RemoveAndDeleteMaterial( const std::string& materialName );
	//Material( const VertexDefinition& vertexDefinition, unsigned int passCount, std::vector<MaterialPass> passes );
	static void DeleteMaterials();

	const std::string& GetName() const { return m_materialName; }
	void Bind();
	void Unbind();
	bool BindInputAttribute( const std::string& attributeNameVerbatim, unsigned int count, VertexFieldType engineFieldType, bool normalize, int strideInBytes, int offsetInBytes );
	
//	const VertexDefinition* GetVertexDefinition() const { return m_vertexDefinition; } //USE THE MESH TO GET THE CURRENT VDEFN INSTEAD.

	//Just calls the ShaderProgram equivalents.
	void SetInt( const std::string& uniformNameVerbatim, int* newValue, unsigned int arraySize = 1 );
	void SetFloat( const std::string& uniformNameVerbatim, float* newValue, unsigned int arraySize = 1 );
	void SetVector2( const std::string& uniformNameVerbatim, const Vector2f* newValue, unsigned int arraySize = 1 );
	void SetVector3( const std::string& uniformNameVerbatim, const Vector3f* newValue, unsigned int arraySize = 1 );
	void SetVector4( const std::string& uniformNameVerbatim, const Vector4f* newValue, unsigned int arraySize = 1 );
	void SetMatrix4x4( const std::string& uniformNameVerbatim, bool shouldTranspose, const Matrix4x4f* newValue, unsigned int arraySize = 1 );
	void SetColor( const std::string& uniformNameVerbatim, const Rgba* newValue, unsigned int arraySize = 1 );
	void SetSampler( const std::string& uniformNameVerbatim, unsigned int newSamplerID );
	void SetTexture( const std::string& uniformNameVerbatim, unsigned int newTextureID );


private:

	Material( const std::string& materialName, const RenderState& renderState, const VertexDefinition& vertexDefinition, ShaderProgram* shaderProgramName );
	static MaterialRegistryMap s_materialRegistry;

	const std::string m_materialName;
	const VertexDefinition* m_vertexDefinition;

	ShaderProgram* m_shaderProgram;
	RenderState m_renderState;
	//unsigned int m_passCount;
	//std::vector<MaterialPass> m_passes;
};
