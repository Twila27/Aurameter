#pragma once


#include <vector>
#include <set>
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/VertexDefinition.hpp"
#include "Engine/Memory/UntrackedAllocator.hpp"
#include "Engine/EngineCommon.hpp"


//--------------------------------------------------------------------------------------------------------------
class VertexDefinition;
struct Rgba;
class Sampler;
class Texture;


//--------------------------------------------------------------------------------------------------------------
enum ShaderVariableType
{
	GLSL_INT,
	GLSL_UINT,
	GLSL_FLOAT,
	GLSL_UVEC2,
	GLSL_UVEC3,
	GLSL_UVEC4,
	GLSL_VEC2,
	GLSL_VEC3,
	GLSL_VEC4,
	GLSL_MAT4,
	GLSL_SAMPLER2D,
	INVALID_TYPE
};


//--------------------------------------------------------------------------------------------------------------
struct ShaderProgramInputAttribute
{
	std::string m_attributeNameVerbatim;
	unsigned int m_attributeBindPointLocation;
	ShaderVariableType m_glslType; //enum to add to Shader.cpp
	bool operator<( const ShaderProgramInputAttribute& compareTo ) const { return m_attributeNameVerbatim < compareTo.m_attributeNameVerbatim; }
};


//--------------------------------------------------------------------------------------------------------------
struct ShaderProgramUniform
{
	const char* m_uniformNameVerbatim;
	unsigned int m_uniformLocation;
	ShaderVariableType m_glslType; //e.g. to tell to use glUniformf, 1f, fv, iv, etc. esp. if it's a sampler2D or not.
	unsigned int m_textureID; //Zero == not a sampler2D variable. Lowest texture ID == 1 == TheRenderer::m_defaultTexture.
	unsigned int m_samplerID; //Zero == not a sampler2D variable. Lowest sampler ID == 1 == TheRenderer::m_defaultSampler.
	unsigned int m_count; // "For arrays, this is the length of the array. For non-arrays, this is 1." NOT e.g. 4 for vec4.
	static const unsigned int UNSET = 0;
};


//--------------------------------------------------------------------------------------------------------------
class ShaderProgram;
typedef std::pair< std::string, ShaderProgram* > ShaderProgramRegistryPair;
typedef std::map< std::string, ShaderProgram*, std::less<std::string>, UntrackedAllocator<ShaderProgramRegistryPair> > ShaderProgramRegistryMap;


//--------------------------------------------------------------------------------------------------------------
class ShaderProgram
{
public:

	static ShaderProgram* CreateOrGetShaderProgram( const std::string& shaderProgramName, const char* vertexShaderFilePath = nullptr, const char* fragmentShaderFilePath = nullptr, const VertexDefinition* vertexDefinition = nullptr );
	static ShaderProgram* CreateOrGetShaderProgram( const std::string& shaderProgramName, Shader* vertexShader, Shader* fragmentShader, const VertexDefinition* vertexDefinition );
	static const ShaderProgramRegistryMap* GetRegistry() { return &s_shaderProgramRegistry; }
	static const char* GetDefaultShaderNameForVertexDefinition( const VertexDefinition* vdefn );
	static void DeleteShaderPrograms();

//	void RecordInputAttributes(); //Handled by Material::BindInputAttribute() as called by MeshRenderer::SetMaterial() as it needs the MR's VAO ID.
	void RecordUniforms();

	~ShaderProgram();

	unsigned int BindInputAttribute( const std::string& attributeNameVerbatim, unsigned int count, VertexFieldType fieldType, bool normalize, int strideInBytes, int offsetInBytes );
	void AddInputAttribute( const std::string& attributeNameVerbatim, unsigned int attributeLocation, ShaderVariableType glslType );

	unsigned int GetShaderProgramID() const { return m_shaderProgramID; }
	const VertexDefinition& GetVertexDefinition() const { return *m_vertexDefinition; }

	void BindProgram();
	void BindTextures();
	static void UnbindAnyPrograms();
	ShaderProgramUniform* FindUniform( const std::string& uniformNameVerbatim );
	bool SetInt( const std::string& uniformNameVerbatim, const int* newValue, unsigned int arraySize = 1 );
	bool SetFloat( const std::string& uniformNameVerbatim, const float* newValue, unsigned int arraySize = 1 );
	bool SetVector2( const std::string& uniformNameVerbatim, const Vector2f* newValue, unsigned int arraySize = 1 );
	bool SetVector3( const std::string& uniformNameVerbatim, const Vector3f* newValue, unsigned int arraySize = 1 );
	bool SetVector4( const std::string& uniformNameVerbatim, const Vector4f* newValue, unsigned int arraySize = 1 );
	bool SetMatrix4x4( const std::string& uniformNameVerbatim, bool shouldTranspose, const Matrix4x4f* newValue, unsigned int arraySize = 1 );
	bool SetColor( const std::string& uniformNameVerbatim, const Rgba* newValue, unsigned int arraySize = 1 );
	bool SetSampler( const std::string& uniformNameVerbatim, unsigned int newSamplerID );
	bool SetTexture( const std::string& uniformNameVerbatim, unsigned int newTextureID );


private:

	ShaderProgram( const std::string& shaderProgramName, 
				   const char* vertexShaderFilePath,
				   const char* fragmentShaderFilePath,
				   const VertexDefinition& vertexDefinition );
	ShaderProgram( const std::string& shaderProgramName, 
				   Shader* vertexShader, 
				   Shader* fragmentShader, 
				   const VertexDefinition& vertexDefinition );
	static ShaderProgramRegistryMap s_shaderProgramRegistry;

	const std::string m_shaderProgramName;
	unsigned int m_shaderProgramID;
	const VertexDefinition* m_vertexDefinition; //For use in accessing attribute data in the shader program.
	std::set<ShaderProgramInputAttribute> m_programInputAttributes;
	std::vector<ShaderProgramUniform> m_programUniforms;
	
	unsigned int m_vertexShaderID;
	unsigned int m_fragmentShaderID;
};