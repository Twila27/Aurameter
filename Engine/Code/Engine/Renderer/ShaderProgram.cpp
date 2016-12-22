#include "Engine/Renderer/ShaderProgram.hpp"


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#include "Engine/Renderer/OpenGLExtensions.hpp"

#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/EngineCommon.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Renderer/Rgba.hpp"
#include "Engine/Renderer/Vertexes.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC ShaderProgramRegistryMap	ShaderProgram::s_shaderProgramRegistry;


//--------------------------------------------------------------------------------------------------------------
ShaderVariableType GetEngineShaderVariableType( GLenum glShaderVariableType )
{
	switch ( glShaderVariableType )
	{
	case GL_UNSIGNED_INT: return GLSL_UINT;
	case GL_INT: return GLSL_INT;
	case GL_FLOAT: return GLSL_FLOAT;
	case GL_UNSIGNED_INT_VEC2: return GLSL_UVEC2;
	case GL_UNSIGNED_INT_VEC3: return GLSL_UVEC3;
	case GL_UNSIGNED_INT_VEC4: return GLSL_UVEC4;
	case GL_FLOAT_VEC2: return GLSL_VEC2;
	case GL_FLOAT_VEC3: return GLSL_VEC3;
	case GL_FLOAT_VEC4: return GLSL_VEC4;
	case GL_FLOAT_MAT4: return GLSL_MAT4;
	case GL_SAMPLER_2D: return GLSL_SAMPLER2D;
	default: ERROR_AND_DIE( "Unsupported glShaderVariableType in GetEngineShaderVariableType!" );
	}
}


//--------------------------------------------------------------------------------------------------------------
STATIC ShaderProgram* ShaderProgram::CreateOrGetShaderProgram( const std::string& shaderProgramName, const char* vertexShaderFilePath, const char* fragmentShaderFilePath, const VertexDefinition* vertexDefinition )
{
	if ( s_shaderProgramRegistry.find( shaderProgramName ) != s_shaderProgramRegistry.end() )
		return s_shaderProgramRegistry[ shaderProgramName ];

	bool hasAllArguments = ( vertexShaderFilePath != nullptr && fragmentShaderFilePath != nullptr && vertexDefinition != nullptr );
	ASSERT_OR_DIE( hasAllArguments, "CreateOrGetShaderProgram caught attempt to make shader program without all arguments!" );
	if ( !hasAllArguments )
		return nullptr;
	
	s_shaderProgramRegistry[ shaderProgramName ] = new ShaderProgram( shaderProgramName, vertexShaderFilePath, fragmentShaderFilePath, *vertexDefinition );
	return s_shaderProgramRegistry[ shaderProgramName ];
}


//--------------------------------------------------------------------------------------------------------------
STATIC ShaderProgram* ShaderProgram::CreateOrGetShaderProgram( const std::string& shaderProgramName, Shader* vertexShader, Shader* fragmentShader, const VertexDefinition* vertexDefinition )
{
	if ( s_shaderProgramRegistry.find( shaderProgramName ) != s_shaderProgramRegistry.end() )
		return s_shaderProgramRegistry[ shaderProgramName ];

	bool hasAllArguments = ( vertexShader != nullptr && fragmentShader != nullptr && vertexDefinition != nullptr );
	ASSERT_OR_DIE( hasAllArguments, "CreateOrGetShaderProgram caught attempt to make shader program without all arguments!" );
	if ( !hasAllArguments )
		return nullptr;

	s_shaderProgramRegistry[ shaderProgramName ] = new ShaderProgram( shaderProgramName, vertexShader, fragmentShader, *vertexDefinition );
	return s_shaderProgramRegistry[ shaderProgramName ];
}


//--------------------------------------------------------------------------------------------------------------
void ReportShaderProgramLinkError( const char* filePath, const char* errorReportContent )
{
	char fullFilePath[ _MAX_PATH ];
	_fullpath( fullFilePath, filePath, _MAX_PATH );

	if ( fullFilePath == NULL )
		ERROR_RECOVERABLE( "_fullpath Macro Failed in ReportShaderProgramLinkError" );

	//Just prints out the fragment shader filepath and line #0 for shader programs.
	std::string dialogContent = "\n-- GLSL Shader Program Link Error Report -- \n";
	dialogContent += fullFilePath;
	dialogContent += "\n\n---------- Version Details Follow ----------\n\n";
	dialogContent += "\nOpenGL Vendor: ";
	dialogContent += (const char*)glGetString( GL_VENDOR );
	dialogContent += "\nOpenGL Version: ";
	dialogContent += (const char*)glGetString( GL_VERSION );
	dialogContent += "\nGLSL Version: ";
	dialogContent += (const char*)glGetString( GL_SHADING_LANGUAGE_VERSION );
	dialogContent += "\n\n---------- Error Log Contents Follow ----------\n\n";
	dialogContent += errorReportContent;
	ERROR_RECOVERABLE( dialogContent.c_str() );
}


//--------------------------------------------------------------------------------------------------------------
GLuint CreateAndLinkProgram( GLuint vertexShader, GLuint fragmentShader, const std::string& fragmentShaderPath ) //Linking a program, really.
{

	GLuint programId = glCreateProgram(); //Making something we can start binding to.
	ASSERT_OR_DIE( programId != NULL, "Error in TheRenderer::CreateAndLinkProgram's glCreateProgram" );

	//Right now the shader files are like the .obj C++ files, and you're putting them together to be optimized.
	glAttachShader( programId, vertexShader );
	glAttachShader( programId, fragmentShader );

	glLinkProgram( programId );

	GLint status;
	glGetProgramiv( programId, GL_LINK_STATUS, &status );
	if ( status == GL_FALSE )
	{
		GLint logLength;
		glGetProgramiv( programId, GL_INFO_LOG_LENGTH, &logLength );

		char* errorBuffer = new char[ logLength + 1 ];
		glGetProgramInfoLog( programId, logLength, &logLength, errorBuffer ); //Load into buffer.

		ReportShaderProgramLinkError( fragmentShaderPath.c_str(), errorBuffer );

		delete errorBuffer;
		glDeleteProgram( programId );
		return NULL;
	}
	//Kept because some shaders can get reused between programs.
	// 	else
	// 	{ //If you don't detach, keeps refs to shader objects, so it lets you use KBs of memory for other things.
	// 		glDetachShader( programId, vertexShader );
	// 		glDetachShader( programId, fragmentShader );
	// 	}

	return programId;
}


//--------------------------------------------------------------------------------------------------------------
ShaderProgram::ShaderProgram( const std::string& shaderProgramName, const char* vertexShaderFilePath, const char* fragmentShaderFilePath, const VertexDefinition& vertexDefinition )
	: m_vertexDefinition( &vertexDefinition )
	, m_shaderProgramName( shaderProgramName )
{
	Shader* vertexShader = Shader::CreateOrGetShader( vertexShaderFilePath, ShaderType::VERTEX_SHADER );
	Shader* fragmentShader = Shader::CreateOrGetShader( fragmentShaderFilePath, ShaderType::FRAGMENT_SHADER );
	ASSERT_OR_DIE( vertexShader != NULL && fragmentShader != NULL, "Found Null Shaders in CreateLoadAndLinkShaderProgram()" );

	unsigned int vertexShaderID = vertexShader->GetID();
	unsigned int fragmentShaderID = fragmentShader->GetID();

	m_shaderProgramID = CreateAndLinkProgram( vertexShaderID, fragmentShaderID, fragmentShaderFilePath );
	ASSERT_OR_DIE( m_shaderProgramID != NULL, "Found Null Program in CreateLoadAndLinkShaderProgram()" );

	//Now that we have the program, no need for the separate shaders--except that other shader programs can use them.
//	glDeleteShader( vertexShaderID );
//	glDeleteShader( fragmentShaderID );
	m_vertexShaderID = vertexShaderID;
	m_fragmentShaderID = fragmentShaderID; //Kept for cleanup in dtor.

	RecordUniforms();

	
	//NOTE: shader input variable attributes are adapted to the GPU program by SetMaterial's calls to BindInputAttribute and AddInputAttribute below.

	//But why not just pull from VertexDefinition here? BECAUSE NEEDS VAO ID TO BIND TO!
}


//--------------------------------------------------------------------------------------------------------------
ShaderProgram::ShaderProgram( const std::string& shaderProgramName, Shader* vertexShader, Shader* fragmentShader, const VertexDefinition& vertexDefinition )
	: m_vertexDefinition( &vertexDefinition )
	, m_shaderProgramName( shaderProgramName )
{
	ASSERT_OR_DIE( vertexShader != NULL && fragmentShader != NULL, "Found Null Shaders in CreateLoadAndLinkShaderProgram()" );

	unsigned int vertexShaderID = vertexShader->GetID();
	unsigned int fragmentShaderID = fragmentShader->GetID();

	m_shaderProgramID = CreateAndLinkProgram( vertexShaderID, fragmentShaderID, "Created From Raw Source" );
	ASSERT_OR_DIE( m_shaderProgramID != NULL, "Found Null Program in CreateLoadAndLinkShaderProgram()" );

	//Now that we have the program, no need for the separate shaders--except that other shader programs can use them.
	//	glDeleteShader( vertexShaderID );
	//	glDeleteShader( fragmentShaderID );
	m_vertexShaderID = vertexShaderID;
	m_fragmentShaderID = fragmentShaderID; //Kept for cleanup in dtor.

	RecordUniforms();


	//NOTE: shader input variable attributes are adapted to the GPU program by SetMaterial's calls to BindInputAttribute and AddInputAttribute below.

	//But why not just pull from VertexDefinition here? BECAUSE NEEDS VAO ID TO BIND TO!
}


//--------------------------------------------------------------------------------------------------------------
STATIC const char* ShaderProgram::GetDefaultShaderNameForVertexDefinition( const VertexDefinition* vdefn )
{
	if ( nullptr == vdefn )
		return nullptr;

	if ( vdefn == &Vertex3D_PCT::DEFINITION )
		return "BlinnPhong_PCT";

	if ( vdefn == &Vertex3D_PCUTB::DEFINITION )
		return "BlinnPhongTBN_PCUTB";

	if ( vdefn == &Vertex3D_Superset::DEFINITION )
		return "BlinnPhongTBN_Superset";
		//return "Skinweights_Superset";

	ERROR_AND_DIE( "Unsupported ShaderProgram::GetDefaultShaderNameForVertexDefinition Request!" );
}


//--------------------------------------------------------------------------------------------------------------
void ShaderProgram::DeleteShaderPrograms()
{
	for ( ShaderProgramRegistryPair shaderProgramPair : s_shaderProgramRegistry )
	{
		if ( shaderProgramPair.second != nullptr )
		{
			delete shaderProgramPair.second;
			shaderProgramPair.second = nullptr;
		}
	}
	s_shaderProgramRegistry.clear();
}


//--------------------------------------------------------------------------------------------------------------
void ShaderProgram::RecordUniforms()
{
	GLint m_numActiveUniforms;
	glGetProgramiv( m_shaderProgramID, GL_ACTIVE_UNIFORMS, &m_numActiveUniforms );
	m_programUniforms.reserve( m_numActiveUniforms );
	for ( int uniformIndex = 0; uniformIndex < m_numActiveUniforms; uniformIndex++ )
	{
		GLsizei bufferSize;
		glGetProgramiv( m_shaderProgramID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &bufferSize );

		GLsizei lengthWritten;
		GLint uniformSize;
		GLenum uniformType;
		char* uniformNameBuffer = new char[ bufferSize ];
		glGetActiveUniform( m_shaderProgramID, uniformIndex, bufferSize, &lengthWritten, &uniformSize, &uniformType, uniformNameBuffer );

		ShaderProgramUniform spu;
		spu.m_glslType = GetEngineShaderVariableType( uniformType );
		spu.m_count = uniformSize;
		spu.m_uniformLocation = uniformIndex;
		spu.m_uniformNameVerbatim = uniformNameBuffer;
		spu.m_samplerID = ShaderProgramUniform::UNSET;
		spu.m_textureID = ShaderProgramUniform::UNSET;
		m_programUniforms.push_back( spu );
	}
}


//--------------------------------------------------------------------------------------------------------------
ShaderProgram::~ShaderProgram()
{
	glDetachShader( m_shaderProgramID, m_vertexShaderID );
	glDetachShader( m_shaderProgramID, m_fragmentShaderID );
	glDeleteShader( m_vertexShaderID );
	glDeleteShader( m_fragmentShaderID );
	glDeleteProgram( m_shaderProgramID );

	for ( ShaderProgramUniform& programUniform : m_programUniforms )
	{
		if ( programUniform.m_uniformNameVerbatim != nullptr )
		{
			delete programUniform.m_uniformNameVerbatim;
			programUniform.m_uniformNameVerbatim = nullptr;
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
TODO( "Revise attribute and uniform caching to Forseth's comments." );
//Called by Material::BindInputAttribute which is called by MeshRenderer::SetMaterial.
unsigned int ShaderProgram::BindInputAttribute( const std::string& attributeName, unsigned int count, VertexFieldType fieldType, bool normalize, int strideInBytes, int offsetInBytes )
{
	int bindPoint = glGetAttribLocation( m_shaderProgramID, attributeName.c_str() );
	unsigned int glFieldType = GetOpenGLVertexFieldType( fieldType );

	if ( bindPoint >= 0 ) //-1 if it doesn't find it, e.g. problems finding the position as a vertex attribute among the rest.
	{
		glEnableVertexAttribArray( bindPoint ); //BE SURE A VAO IS BEING BOUND AROUND THIS OR GLDRAW THROWS ERRORS!

		if ( fieldType != VERTEX_FIELD_TYPE_UNSIGNED_INT )
			glVertexAttribPointer(
				bindPoint,  // Bind port to shader
				count, // Number of array elements being passed in -- works even with a vec4 being passed in.
				(GLenum)glFieldType, // What type each element within a field can be expected to be, e.g. GL_FLOAT could be 1 or 4 floats
				(GLboolean)normalize, // Whether this data is normalized, if it's GL_TRUE, it maps the value to [0,1], e.g. good for normals.
				(GLsizei)strideInBytes, // Stride: how far between each the beginning of each element
				(GLvoid*)offsetInBytes // Where we can find the first such attribute in the buffer. Returns # bytes from beginning of type.
			);
		else
			glVertexAttribIPointer(
				bindPoint,  // Bind port to shader
				count, // Number of array elements being passed in -- works even with a vec4 being passed in.
				(GLenum)glFieldType, // What type each element within a field can be expected to be, e.g. GL_FLOAT could be 1 or 4 floats
				(GLsizei)strideInBytes, // Stride: how far between each the beginning of each element
				(GLvoid*)offsetInBytes // Where we can find the first such attribute in the buffer. Returns # bytes from beginning of type.
			);
	}

	return bindPoint;
}


//--------------------------------------------------------------------------------------------------------------
TODO( "Revise attribute and uniform caching to Forseth's comments." );
//Called by Material::BindInputAttribute which is called by MeshRenderer::SetMaterial.
void ShaderProgram::AddInputAttribute( const std::string& attributeNameVerbatim, unsigned int attributeLocation, ShaderVariableType glslType )
{
	ShaderProgramInputAttribute attr;
	attr.m_attributeNameVerbatim = attributeNameVerbatim;
	attr.m_attributeBindPointLocation = attributeLocation;
	attr.m_glslType = glslType;

	m_programInputAttributes.insert( attr );
}


//--------------------------------------------------------------------------------------------------------------
void ShaderProgram::BindProgram()
{
	glUseProgram( m_shaderProgramID );
}


//--------------------------------------------------------------------------------------------------------------
void ShaderProgram::BindTextures()
{
	//Bind textures to slots, looping over all uniforms to see which ones are textures.
	unsigned int texIndex = 0;
	for ( ShaderProgramUniform currentUniform : m_programUniforms )
	{
		if ( currentUniform.m_glslType != GLSL_SAMPLER2D )
			continue;

		if ( currentUniform.m_samplerID == ShaderProgramUniform::UNSET
			 || currentUniform.m_textureID == ShaderProgramUniform::UNSET )
			continue;

		glActiveTexture( GL_TEXTURE0 + texIndex );
		glBindTexture( GL_TEXTURE_2D, currentUniform.m_textureID );
		glBindSampler( texIndex, currentUniform.m_samplerID );
		SetInt( currentUniform.m_uniformNameVerbatim, (int*)&texIndex ); //The active tex bind port from glActiveTexture, not the tex ID.
																  //NOT passing the samplerID, passing where to bind the sampler to, since OpenGL has a limited # sampler ports to bind at.

		++texIndex;
	}
}


//--------------------------------------------------------------------------------------------------------------
STATIC void ShaderProgram::UnbindAnyPrograms()
{
	glUseProgram( NULL );
	glActiveTexture( GL_TEXTURE0 ); //Else font will mess up and all objects will texture oddly (since it doesn't reset to default plain white texture).
}


//--------------------------------------------------------------------------------------------------------------
ShaderProgramUniform* ShaderProgram::FindUniform( const std::string& uniformNameVerbatim )
{
	for ( ShaderProgramUniform& currentUniform : m_programUniforms )
	{
		if ( currentUniform.m_uniformNameVerbatim != uniformNameVerbatim )
			continue;

		return &currentUniform;
	}
	return nullptr;
}


//--------------------------------------------------------------------------------------------------------------
bool ShaderProgram::SetMatrix4x4( const std::string& uniformNameVerbatim, bool shouldTranspose, const Matrix4x4f* val, unsigned int arraySize /*= 1*/ )
{
	if ( FindUniform( uniformNameVerbatim ) == nullptr )
		return false;

	FIXME( "Remove m_ordering => GetMatrixOrdering { return ROW_MAJOR } depending on GL ES. This slows down things by remaking vectors repeatedly." );
	//Convert Matrix4x4 class which has extra members to an array of only matrix data to send to card:
	std::vector< float > matrixValues;
	for ( unsigned int matrixIndex = 0; matrixIndex < arraySize; matrixIndex++ )
	{
		const float* inMatrixData = val[ matrixIndex ].m_data;
		for ( int i = 0; i < 16; i++ )
			matrixValues.push_back( inMatrixData[ i ] );
	}

	BindProgram();
	GLint loc = glGetUniformLocation( m_shaderProgramID, uniformNameVerbatim.c_str() );
	if ( loc >= 0 )
	{
		TODO( "Replace &val by _offsetof(Matrix4x4, m_data) to foolproof against shifting bug." );
		//BE VERY CAREFUL THAT THE DATA IS THE FIRST ELEMENT OF THE CLASS, ELSE BELOW CODE IS SHIFTED BY OTHER MEMBERS OF MATRIX CLASS.
		glUniformMatrix4fv( loc, arraySize, (GLboolean)shouldTranspose, (GLfloat*)matrixValues.data() ); //shouldTranspose: must be true if using row-major MVP && left-multiplying vert*MVP in shader!.
		//NOTE: OpenGL ES and WebGL seem to potentially not allow true here, in which case just invert any val param with val->m_ordering == ROW_MAJOR.
		UnbindAnyPrograms();
		return true;
	}
	UnbindAnyPrograms();
	return false;
}


//--------------------------------------------------------------------------------------------------------------
bool ShaderProgram::SetColor( const std::string& uniformNameVerbatim, const Rgba* val, unsigned int arraySize /*= 1*/ )
{
	if ( FindUniform( uniformNameVerbatim ) == nullptr )
		return false;

	BindProgram();
	GLint loc = glGetUniformLocation( m_shaderProgramID, uniformNameVerbatim.c_str() );
	if ( loc >= 0 )
	{
		Vector4f* colorAsFloats = new Vector4f[ arraySize ];
		for ( unsigned int colorArrayIndex = 0; colorArrayIndex < arraySize; colorArrayIndex++ )
			colorAsFloats[ colorArrayIndex ] = val[ colorArrayIndex ].GetAsFloats();

		//Need to loop and do this cast based on the arraySize per array element!!

		glUniform4fv( loc, arraySize, (GLfloat*)&colorAsFloats[0] );
		UnbindAnyPrograms();
		delete colorAsFloats;
		return true;
	}
	UnbindAnyPrograms();
	return false;
}


//--------------------------------------------------------------------------------------------------------------
bool ShaderProgram::SetVector2( const std::string& uniformNameVerbatim, const Vector2f* val, unsigned int arraySize /*= 1*/ )
{
	if ( FindUniform( uniformNameVerbatim ) == nullptr )
		return false;

	BindProgram();
	GLint loc = glGetUniformLocation( m_shaderProgramID, uniformNameVerbatim.c_str() );
	if ( loc >= 0 )
	{
		glUniform2fv( loc, arraySize, (GLfloat*)val );
		UnbindAnyPrograms();
		return true;
	}
	UnbindAnyPrograms();
	return false;
}


//--------------------------------------------------------------------------------------------------------------
bool ShaderProgram::SetVector3( const std::string& uniformNameVerbatim, const Vector3f* val, unsigned int arraySize /*= 1*/ )
{
	if ( FindUniform( uniformNameVerbatim ) == nullptr )
		return false;

	BindProgram();
	GLint loc = glGetUniformLocation( m_shaderProgramID, uniformNameVerbatim.c_str() );
	if ( loc >= 0 )
	{
		glUniform3fv( loc, arraySize, (GLfloat*)val );
		UnbindAnyPrograms();
		return true;
	}
	UnbindAnyPrograms();
	return false;
}


//--------------------------------------------------------------------------------------------------------------
bool ShaderProgram::SetVector4( const std::string& uniformNameVerbatim, const Vector4f* val, unsigned int arraySize /*= 1*/ )
{
	if ( FindUniform( uniformNameVerbatim ) == nullptr )
		return false;

	BindProgram();
	GLint loc = glGetUniformLocation( m_shaderProgramID, uniformNameVerbatim.c_str() );
	if ( loc >= 0 )
	{
		glUniform4fv( loc, arraySize, (GLfloat*)val );
		UnbindAnyPrograms();
		return true;
	}
	UnbindAnyPrograms();
	return false;
}


//--------------------------------------------------------------------------------------------------------------
bool ShaderProgram::SetFloat( const std::string& uniformNameVerbatim, const float* val, unsigned int arraySize /*= 1*/ )
{
	if ( FindUniform( uniformNameVerbatim ) == nullptr )
		return false;

	BindProgram();
	GLint loc = glGetUniformLocation( m_shaderProgramID, uniformNameVerbatim.c_str() );
	if ( loc >= 0 )
	{
		glUniform1fv( loc, arraySize, (GLfloat*)val );
		UnbindAnyPrograms();
		return true;
	}
	UnbindAnyPrograms();
	return false;
}


//--------------------------------------------------------------------------------------------------------------
bool ShaderProgram::SetInt( const std::string& uniformNameVerbatim, const int* val, unsigned int arraySize /*= 1*/ ) //Also used for setting a sampler value.
{
	if ( FindUniform( uniformNameVerbatim ) == nullptr )
		return false;

	BindProgram();
	GLint loc = glGetUniformLocation( m_shaderProgramID, uniformNameVerbatim.c_str() );
	if ( loc >= 0 )
	{
		glUniform1iv( loc, arraySize, (GLint*)val );
		UnbindAnyPrograms();
		return true;
	}
	UnbindAnyPrograms();
	return false;
}


//--------------------------------------------------------------------------------------------------------------
bool ShaderProgram::SetSampler( const std::string& uniformNameVerbatim, unsigned int newSamplerID ) //Also used for setting a sampler value.
{
	if ( FindUniform( uniformNameVerbatim ) == nullptr )
		return false;

	//Edit m_programUniforms, let Material::Render() call BindProgram() which can then loop and handle textures.
	ShaderProgramUniform* requestedUniform = FindUniform( uniformNameVerbatim );
	if ( requestedUniform == nullptr )
		return false;

	if ( requestedUniform->m_glslType != GLSL_SAMPLER2D )
		return false;

	requestedUniform->m_samplerID = newSamplerID;

	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool ShaderProgram::SetTexture( const std::string& uniformNameVerbatim, unsigned int newTextureID ) //Also used for setting a sampler value.
{
	if ( FindUniform( uniformNameVerbatim ) == nullptr )
		return false;

	//Edit m_programUniforms, let Material::Render() call BindProgram() which can then loop all uniforms, use .m_texIndex > 0 to see which are textures.
	ShaderProgramUniform* requestedUniform = FindUniform( uniformNameVerbatim );
	if ( requestedUniform == nullptr )
		return false;

	if ( requestedUniform->m_glslType != GLSL_SAMPLER2D )
		return false;

	requestedUniform->m_textureID = newTextureID;

	return true;
}
