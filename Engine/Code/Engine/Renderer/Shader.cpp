#pragma warning( disable: 4996 ) // 'fopen': This function or variable may be unsafe. Consider using fopen_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.

#include "Engine/Renderer/Shader.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/EngineCommon.hpp"
#include "Engine/FileUtils/FileUtils.hpp"
#include "Engine/String/StringUtils.hpp"


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#include "Engine/Renderer/OpenGLExtensions.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC ShaderRegistryMap Shader::s_shaderRegistry;


//--------------------------------------------------------------------------------------------------------------
unsigned int GetOpenGLShaderType( ShaderType engineShaderType )
{
	switch ( engineShaderType )
	{
	case ShaderType::VERTEX_SHADER: return GL_VERTEX_SHADER;
	case ShaderType::FRAGMENT_SHADER: return GL_FRAGMENT_SHADER;
	case ShaderType::INVALID_SHADER: ERROR_AND_DIE( "Invalid ShaderType in GetOpenGLShaderType!" );
	default: ERROR_AND_DIE( "Unsupported ShaderType in GetOpenGLShaderType!" );
	}
}


//--------------------------------------------------------------------------------------------------------------
//Non-member for use by static CreateOrGet function.
void ReportShaderCompileError( const char* filePath, const char* errorReportContent )
{
	char fullFilePath[ _MAX_PATH ];
	_fullpath( fullFilePath, filePath, _MAX_PATH );

	if ( fullFilePath == NULL )
		ERROR_RECOVERABLE( "_fullpath Macro Failed in ReportShaderCompileError" );

	//Assumes GLSL shader compilation errors begin new lines with "0(".
	std::string insertSubstr = fullFilePath;
	insertSubstr += '(';
	const std::string errorReportContentWithFullFilePath = ReplaceAllOccurrencesInStringOfSubstr( errorReportContent, "0(", insertSubstr ).c_str();

	std::string dialogContent = "\n-- GLSL Shader Compile Error Report -- \n";
	dialogContent += fullFilePath;
	dialogContent += "\n\n---------- Version Details Follow ----------\n\n";
	dialogContent += "\nOpenGL Vendor: ";
	dialogContent += (const char*)glGetString( GL_VENDOR );
	dialogContent += "\nOpenGL Version: ";
	dialogContent += (const char*)glGetString( GL_VERSION );
	dialogContent += "\nGLSL Version: ";
	dialogContent += (const char*)glGetString( GL_SHADING_LANGUAGE_VERSION );
	dialogContent += "\n\n---------- Error Log Contents Follow ----------\n\n";
	dialogContent += errorReportContentWithFullFilePath;
	ERROR_RECOVERABLE( dialogContent.c_str() );
}


//--------------------------------------------------------------------------------------------------------------
GLint LoadShader( GLenum shaderType, GLint sourceBufferLength, GLchar* bufferAsPacifyingGL, const char* filepathToReportOnError = nullptr )
{
	//Get-register handle for shader.
	GLuint shaderId = glCreateShader( shaderType );
	ASSERT_OR_DIE( shaderId != NULL, "Error in TheRenderer::LoadShader's glCreateShader" );

	//Assign source to handle.
	GLint srcLength = sourceBufferLength;
	glShaderSource( shaderId, 1, &bufferAsPacifyingGL, &srcLength ); //"This program goes with this shader."
		//Can bind as many buffers/files to a shader as you want (the "1").
		//Treated as a giant .cpp of all supplied files.
		//Shaders don't support #include, would have to make your own preprocessing code to effect that.

	//Compile the shader.
	glCompileShader( shaderId );

	//Error check.
	GLint status;
	glGetShaderiv( shaderId, GL_COMPILE_STATUS, &status );
	if ( status == GL_FALSE )
	{
		GLint length;
		glGetShaderiv( shaderId, GL_INFO_LOG_LENGTH, &length ); //Get buffer length.

		char* errorBuffer = new char[ length + 1 ];
		glGetShaderInfoLog( shaderId, length, &length, errorBuffer ); //Load into buffer.

		ReportShaderCompileError( filepathToReportOnError, errorBuffer );

		delete errorBuffer;
		glDeleteShader( shaderId );
		return 0;
	}

	ASSERT_OR_DIE( status == GL_TRUE, "Error in TheRenderer::LoadShader's glGetShaderiv" );

	return shaderId;
}


//--------------------------------------------------------------------------------------------------------------
//Non-member for use by static CreateOrGet function.
GLint LoadShaderFromFile( const char* filePath, GLenum shaderType )
{
	std::vector< unsigned char > shaderSourceBuffer;
	LoadBinaryFileIntoBuffer( filePath, shaderSourceBuffer ); //FileUtils call.
	GLchar* bufferAsPacifyingGL = (GLchar*)&shaderSourceBuffer[ 0 ]; //Doesn't take a vector, hence pacifying.

	return LoadShader( shaderType, shaderSourceBuffer.size(), bufferAsPacifyingGL, filePath ); //Returns shader ID.
}


//---------------------------------------------------------------------------
// Finds the named Shader among the registry of those already loaded; if
//	found, returns that Shader*.  If not, attempts to load that shader,
//	and returns a Shader* just created (or nullptr if unable to load file).
//
STATIC Shader* Shader::CreateOrGetShader( const std::string& shaderFilePath, ShaderType shaderType )
{
	if ( s_shaderRegistry.find( shaderFilePath ) != s_shaderRegistry.end() )
		return s_shaderRegistry[ shaderFilePath ];

	unsigned int shaderID = LoadShaderFromFile( shaderFilePath.c_str(), GetOpenGLShaderType( shaderType ) );
	s_shaderRegistry.insert( ShaderRegistryPair( shaderFilePath, new Shader( shaderFilePath, shaderType, shaderID ) ) );
	return s_shaderRegistry[ shaderFilePath ];
}


//-----------------------------------------------------------------------------
STATIC Shader* Shader::CreateShaderFromSource( const char* shaderSourceBuffer, unsigned int bufferLength, ShaderType shaderType )
{
	static unsigned int numInvocation = 1;
	//Be forewarned that there is no create-or-get lookup to prevent duplicates.

	unsigned int shaderID = LoadShader( GetOpenGLShaderType( shaderType ), bufferLength, (GLchar*)shaderSourceBuffer  );
	std::string name = Stringf( "CreatedFromRawSource #%d", numInvocation++ );
	s_shaderRegistry.insert( ShaderRegistryPair( name, new Shader( name, shaderType, shaderID ) ) );
	return s_shaderRegistry[ name ];
}



//--------------------------------------------------------------------------------------------------------------
void Shader::DeleteShaders()
{
	for ( ShaderRegistryPair shaderPair : s_shaderRegistry )
	{
		if ( shaderPair.second != nullptr )
		{
			delete shaderPair.second;
			shaderPair.second = nullptr;
		}
	}
	s_shaderRegistry.clear();
}
