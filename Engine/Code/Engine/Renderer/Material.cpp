#include "Engine/Renderer/Material.hpp"


#include "Engine/Renderer/RenderState.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Vertexes.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Matrix4x4.hpp"
#include "Engine/Renderer/Rgba.hpp"
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer/Texture.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC MaterialRegistryMap		Material::s_materialRegistry;


//--------------------------------------------------------------------------------------------------------------
void Material::Bind()
{
	m_shaderProgram->UnbindAnyPrograms();
	m_shaderProgram->BindTextures(); //MUST PRECEDE BIND PROGRAM.
	m_shaderProgram->BindProgram(); //MUST NOT PRECEDE BIND TEXTURES.
	m_renderState.Bind();
}


//--------------------------------------------------------------------------------------------------------------
void Material::Unbind()
{
	m_shaderProgram->UnbindAnyPrograms();
}


//--------------------------------------------------------------------------------------------------------------
Material::Material( const std::string& materialName, const RenderState& renderState, const VertexDefinition& vertexDefinition, 
					ShaderProgram* shaderProgram )
	: m_materialName( materialName )
	, m_renderState( renderState )
	, m_vertexDefinition( &vertexDefinition )
	, m_shaderProgram( shaderProgram )
{
}


//--------------------------------------------------------------------------------------------------------------
STATIC Material* Material::CreateOrGetMaterial( const std::string& materialName, const RenderState* renderState /* = nullptr */, const VertexDefinition* vertexDefinition /* = nullptr */,
										   const char* shaderProgramName /* = nullptr */, const char* vertexShaderFilePath /* = nullptr */, const char* fragmentShaderFilePath /* = nullptr */ )
{
	if ( s_materialRegistry.find( materialName ) != s_materialRegistry.end() )
		return s_materialRegistry[ materialName ];


	bool hasAllArguments = ( renderState != nullptr && vertexDefinition != nullptr && shaderProgramName != nullptr );
	ASSERT_OR_DIE( hasAllArguments, "CreateOrGetMaterial found attempt to make material without all params!" );
	if ( !hasAllArguments )
		return nullptr;

	s_materialRegistry[ materialName ] = new Material( materialName, *renderState, *vertexDefinition, ShaderProgram::CreateOrGetShaderProgram( shaderProgramName, vertexShaderFilePath, fragmentShaderFilePath, vertexDefinition ) );
	return s_materialRegistry[ materialName ];
}


//--------------------------------------------------------------------------------------------------------------
STATIC Material* Material::CreateOrGetMaterial( const std::string& materialName, const RenderState* renderState, const VertexDefinition* vertexDefinition,
												const char* shaderProgramName, Shader* vertexShader, Shader* fragmentShader )
{
	if ( s_materialRegistry.find( materialName ) != s_materialRegistry.end() )
		return s_materialRegistry[ materialName ];


	bool hasAllArguments = ( renderState != nullptr && vertexDefinition != nullptr && shaderProgramName != nullptr );
	ASSERT_OR_DIE( hasAllArguments, "CreateOrGetMaterial found attempt to make material without all params!" );
	if ( !hasAllArguments )
		return nullptr;

	s_materialRegistry[ materialName ] = new Material( materialName, *renderState, *vertexDefinition, ShaderProgram::CreateOrGetShaderProgram( shaderProgramName, vertexShader, fragmentShader, vertexDefinition ) );
	return s_materialRegistry[ materialName ];
}


//--------------------------------------------------------------------------------------------------------------
bool Material::BindInputAttribute( const std::string& attributeNameVerbatim, unsigned int count, VertexFieldType engineFieldType, bool normalize, int strideInBytes, int offsetInBytes )
{
//	if ( m_shaderProgram->FindAttribute( attributeNameVerbatim )  )
	int bindPoint = m_shaderProgram->BindInputAttribute( attributeNameVerbatim, count, engineFieldType, normalize, strideInBytes, offsetInBytes );

	if ( bindPoint < 0 )
		return false;

	//Not a separate function because it's limited to the special use case conditions of input attributes.
	ShaderVariableType varType = ShaderVariableType::INVALID_TYPE;
	switch ( engineFieldType )
	{
	case VERTEX_FIELD_TYPE_UNSIGNED_BYTE: //Because it seems it gets converted to a float by the time it reaches the GPU-side.
	case VERTEX_FIELD_TYPE_FLOAT:
		switch ( count )
		{
		case 1: varType = GLSL_FLOAT; break;
		case 2: varType = GLSL_VEC2;  break;
		case 3: varType = GLSL_VEC3;  break;
		case 4: varType = GLSL_VEC4;  break;
		}
		break;
	case VERTEX_FIELD_TYPE_UNSIGNED_INT:
		switch ( count )
		{
		case 1: varType = GLSL_UINT; break;
		case 2: varType = GLSL_UVEC2; break;
		case 3: varType = GLSL_UVEC3; break;
		case 4: varType = GLSL_UVEC4; break;
		}
		break;
	default: ERROR_AND_DIE("Found unsupported field type in Material::BindInputAttribute!");
	}

	ASSERT_OR_DIE( varType != ShaderVariableType::INVALID_TYPE, "No type assigned to varType in Material::BindInputAttribute!" );
	m_shaderProgram->AddInputAttribute( attributeNameVerbatim, bindPoint, varType );

	return true;
}


//--------------------------------------------------------------------------------------------------------------
void Material::SetInt( const std::string& uniformNameVerbatim, int* newValue, unsigned int arraySize /* = 1 */ )
{
	m_shaderProgram->SetInt( uniformNameVerbatim, newValue, arraySize );
}


//--------------------------------------------------------------------------------------------------------------
void Material::SetFloat( const std::string& uniformNameVerbatim, float* newValue, unsigned int arraySize /* = 1 */ )
{
	m_shaderProgram->SetFloat( uniformNameVerbatim, newValue, arraySize );
}


//--------------------------------------------------------------------------------------------------------------
void Material::SetVector2( const std::string& uniformNameVerbatim, const Vector2f* newValue, unsigned int arraySize /* = 1 */ )
{
	m_shaderProgram->SetVector2( uniformNameVerbatim, newValue, arraySize );
}


//--------------------------------------------------------------------------------------------------------------
void Material::SetVector3( const std::string& uniformNameVerbatim, const Vector3f* newValue, unsigned int arraySize /* = 1 */ )
{
	m_shaderProgram->SetVector3( uniformNameVerbatim, newValue, arraySize );
}


//--------------------------------------------------------------------------------------------------------------
void Material::SetVector4( const std::string& uniformNameVerbatim, const Vector4f* newValue, unsigned int arraySize /* = 1 */ )
{
	m_shaderProgram->SetVector4( uniformNameVerbatim, newValue, arraySize );
}


//--------------------------------------------------------------------------------------------------------------
void Material::SetMatrix4x4( const std::string& uniformNameVerbatim, bool shouldTranspose, const Matrix4x4f* newValue, unsigned int arraySize /* = 1 */ )
{
	m_shaderProgram->SetMatrix4x4( uniformNameVerbatim, shouldTranspose, newValue, arraySize );
}


//--------------------------------------------------------------------------------------------------------------
void Material::SetColor( const std::string& uniformNameVerbatim, const Rgba* newValue, unsigned int arraySize /* = 1 */ )
{
	m_shaderProgram->SetColor( uniformNameVerbatim, newValue, arraySize );
}


//--------------------------------------------------------------------------------------------------------------
void Material::SetSampler( const std::string& uniformNameVerbatim, unsigned int newSamplerID )
{
	m_shaderProgram->SetSampler( uniformNameVerbatim, newSamplerID );
}


//--------------------------------------------------------------------------------------------------------------
void Material::SetTexture( const std::string& uniformNameVerbatim, unsigned int newTextureID )
{
	m_shaderProgram->SetTexture( uniformNameVerbatim, newTextureID );
}


//--------------------------------------------------------------------------------------------------------------
bool Material::RemoveAndDeleteMaterial( const std::string& materialName )
{
	bool didRemove = false;

	MaterialRegistryMap::iterator searchResult = s_materialRegistry.find( materialName );
	if ( searchResult != s_materialRegistry.end() )
	{
		delete searchResult->second;
		s_materialRegistry.erase( searchResult );
		didRemove = true;
	}

	return didRemove;
}


//--------------------------------------------------------------------------------------------------------------
void Material::DeleteMaterials()
{
	for ( MaterialRegistryPair matPair : s_materialRegistry )
	{
		if ( matPair.second != nullptr )
		{
			delete matPair.second;
			matPair.second = nullptr;
		}
	}
	s_materialRegistry.clear();
}
