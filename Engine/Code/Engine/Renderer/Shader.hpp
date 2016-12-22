#pragma once


#include <map>
#include "Engine/Memory/UntrackedAllocator.hpp"
#include "Engine/EngineCommon.hpp"


//--------------------------------------------------------------------------------------------------------------
enum ShaderType
{
	VERTEX_SHADER,
	FRAGMENT_SHADER,
	INVALID_SHADER,
	NUM_SHADER_TYPES
};


//--------------------------------------------------------------------------------------------------------------
class Shader;
typedef std::pair< std::string, Shader* > ShaderRegistryPair;
typedef std::map< std::string, Shader*, std::less<std::string>, UntrackedAllocator<ShaderRegistryPair> > ShaderRegistryMap;


//--------------------------------------------------------------------------------------------------------------
class Shader
{
public:

	static Shader* CreateOrGetShader( const std::string& shaderFilePath, ShaderType shaderType );
	static Shader* CreateShaderFromSource( const char* shaderSourceBuffer, unsigned int bufferLength, ShaderType shaderType );
	static void DeleteShaders();
	inline unsigned int GetID() const {	return m_shaderID; }
	inline unsigned int GetShaderType() const { return m_shaderType; }


private:

	Shader( const std::string& sourceFilePath, ShaderType shaderType, unsigned int shaderID )
		: m_sourceFilePath( sourceFilePath )
		, m_shaderType( shaderType )
		, m_shaderID( shaderID ) 
	{
	}

	static ShaderRegistryMap s_shaderRegistry;
	unsigned int m_shaderID;
	unsigned int m_shaderType;
	const std::string m_sourceFilePath;
};