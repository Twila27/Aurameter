#include "Engine/Renderer/MeshRenderer.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#include "Engine/Renderer/OpenGLExtensions.hpp"

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
STATIC MeshRendererRegistryMap		MeshRenderer::s_meshRendererRegistry;


//--------------------------------------------------------------------------------------------------------------
STATIC MeshRenderer* MeshRenderer::CreateOrGetMeshRenderer( const std::string& meshRendererName, std::shared_ptr<Mesh> mesh, Material* material )
{
	if ( s_meshRendererRegistry.find( meshRendererName ) != s_meshRendererRegistry.end() )
		return s_meshRendererRegistry[ meshRendererName ];


	s_meshRendererRegistry[ meshRendererName ] = new MeshRenderer( mesh, material );
	return s_meshRendererRegistry[ meshRendererName ];
}


//--------------------------------------------------------------------------------------------------------------
STATIC MeshRenderer* MeshRenderer::OverwriteMeshRenderer( const std::string& overwrittenMeshRendererName, std::shared_ptr<Mesh> mesh, Material* material )
{
	ASSERT_OR_DIE( s_meshRendererRegistry.find( overwrittenMeshRendererName ) != s_meshRendererRegistry.end(), "Nothing to Overwrite in OverwriteMeshRenderer!" );

	s_meshRendererRegistry[ overwrittenMeshRendererName ] = new MeshRenderer( mesh, material );
	return s_meshRendererRegistry[ overwrittenMeshRendererName ];
}


//--------------------------------------------------------------------------------------------------------------
STATIC void MeshRenderer::ClearMeshRenderers()
{
	s_meshRendererRegistry.clear();
}


//--------------------------------------------------------------------------------------------------------------
MeshRenderer::MeshRenderer( std::shared_ptr<Mesh> mesh, Material* material )
	: m_material( nullptr )
	, m_mesh( nullptr )
{
	glGenVertexArrays( 1, &m_vaoID );
	ASSERT_OR_DIE( m_vaoID != NULL, "glGenVertexArrays failed in MeshRenderer()" );

	if ( mesh != nullptr )
		SetMesh( mesh );

	if ( material != nullptr )
		SetMaterial( material, true );
}


//--------------------------------------------------------------------------------------------------------------
MeshRenderer::~MeshRenderer()
{
	glDeleteVertexArrays( 1, &m_vaoID );
}


//--------------------------------------------------------------------------------------------------------------
const std::string& MeshRenderer::GetMaterialName() const
{
	return m_material->GetName();
}


//--------------------------------------------------------------------------------------------------------------
const VertexDefinition* MeshRenderer::GetVertexDefinition() const
{
	return m_mesh->GetVertexDefinition();
}


//--------------------------------------------------------------------------------------------------------------
void MeshRenderer::SetMesh( std::shared_ptr<Mesh> mesh )
{
	glBindVertexArray( m_vaoID );
	glBindBuffer( GL_ARRAY_BUFFER, mesh->GetVertexBufferID() );

	if ( mesh->GetIndexBufferID() != NULL )
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mesh->GetIndexBufferID() );

	glBindVertexArray( NULL ); //Must come first, but why?

	glBindBuffer( GL_ARRAY_BUFFER, NULL );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, NULL ); //Why does this cause a crash if before VAO unbind?

	m_mesh = mesh;
}


//--------------------------------------------------------------------------------------------------------------
void MeshRenderer::SetMaterial( Material* material, bool overwriteMemberMaterial )
{
	if ( m_mesh == nullptr ) return;

	glBindVertexArray( m_vaoID );
	glBindBuffer( GL_ARRAY_BUFFER, m_mesh->GetVertexBufferID() );

	if ( m_mesh->UsesIndexBuffer() )
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_mesh->GetIndexBufferID() );
	else
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, NULL );

	for ( int i = 0; i < 10; i++ ) //Being overly cautious, 10 is a random #, replace it with superset's #attribs.
		glDisableVertexAttribArray( i );

	//DO NOT USE the material vdefn, because it doesn't necessarily match to the mesh's.
	const VertexDefinition* vertexDefinition = m_mesh->GetVertexDefinition();
	for ( unsigned int attributeIndex = 0; attributeIndex < vertexDefinition->GetNumAttributes(); attributeIndex++ )
	{
		const VertexAttribute* attr = vertexDefinition->GetAttributeAtIndex( attributeIndex );

		TODO( "Add a function to return correct sizes for different Vertex types, but how to tell which Vertexes' type a definition corresponds to?" );
		if ( !material->BindInputAttribute( attr->m_attributeName.c_str(), attr->m_count, attr->m_fieldType,
										   attr->m_normalized, vertexDefinition->GetVertexSize(), attr->m_offset ) )
		{
//			DebuggerPrintf( "Expected Vertex Attribute %s Not Found in %s by MeshRenderer::SetMaterial!\n\n", attr->m_attributeName.c_str(), material->GetName().c_str() );
		}
	}

	glBindVertexArray( NULL ); //Must come first, but why?

	glBindBuffer( GL_ARRAY_BUFFER, NULL );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, NULL ); //Why does this cause a crash if before VAO unbind?

	if ( overwriteMemberMaterial && ( m_material != material ) )
	{
//		if ( m_material != nullptr )
//			Material::RemoveAndDeleteMaterial( m_material->GetName() );
		//Causing crash with Unity-chan + setrendermode hotswapping...

		m_material = material;
	}
}


//--------------------------------------------------------------------------------------------------------------
void MeshRenderer::Render()
{
	Render( m_mesh.get(), m_material );
}


//--------------------------------------------------------------------------------------------------------------
void MeshRenderer::ResetVAO( Mesh* mesh, Material* material )
{
	UNREFERENCED( mesh );
	TODO( "Cleanup reset code upon FBXLoad in Tools/FBXUtils here!" );

	glDeleteVertexArrays( 1, &m_vaoID );

	glGenVertexArrays( 1, &m_vaoID );
	ASSERT_OR_DIE( m_vaoID != NULL, "glGenVertexArrays failed in MeshRenderer()" );

	SetMesh( m_mesh );

	SetMaterial( material, true );
}


//--------------------------------------------------------------------------------------------------------------
void MeshRenderer::Render( Mesh* mesh, Material* material )
{
	//Ideal: loop over all the instructions with the mesh, draw elements from startIndex to endIndex.

	if ( mesh != m_mesh.get() )
		SetMesh( std::shared_ptr<Mesh>(mesh) );
		 
	if ( material != m_material )
		SetMaterial( material, true );

	glBindVertexArray( m_vaoID );

	if ( m_mesh->UsesIndexBuffer() )
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_mesh->GetIndexBufferID() );
	else
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, NULL );

	material->Bind();

	const std::vector<DrawInstruction>& drawInstructions = m_mesh->GetDrawInstructions();
	for ( unsigned int instructionIndex = 0; instructionIndex < drawInstructions.size(); instructionIndex++ )
	{
		const DrawInstruction& currentInstruction = drawInstructions[ instructionIndex ];

		TODO( "Synthesize functionality with TheRenderer enum!" );
		unsigned int vertexGroupingRule = GetOpenGLVertexGroupingRule( currentInstruction.m_type );

		if ( m_mesh->UsesIndexBuffer() )
			glDrawElements( vertexGroupingRule, currentInstruction.m_count, GL_UNSIGNED_INT, (GLvoid*)currentInstruction.m_startIndex ); //Vertex grouping rule, # indices, uint, &loc[0] or 0 for bound.
		else
			glDrawArrays( vertexGroupingRule, currentInstruction.m_startIndex, currentInstruction.m_count ); //Vertex grouping rule, start index into bound array, # vertexes to include.
	}

	material->Unbind();

	glBindVertexArray( NULL );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, NULL );
}


//--------------------------------------------------------------------------------------------------------------
void MeshRenderer::SetInt( const std::string& uniformNameVerbatim, int* newValue )
{
	m_material->SetInt( uniformNameVerbatim, newValue );
}


//--------------------------------------------------------------------------------------------------------------
void MeshRenderer::SetFloat( const std::string& uniformNameVerbatim, float* newValue )
{
	m_material->SetFloat( uniformNameVerbatim, newValue );
}


//--------------------------------------------------------------------------------------------------------------
void MeshRenderer::SetVector2( const std::string& uniformNameVerbatim, const Vector2f* newValue )
{
	m_material->SetVector2( uniformNameVerbatim, newValue );
}


//--------------------------------------------------------------------------------------------------------------
void MeshRenderer::SetVector3( const std::string& uniformNameVerbatim, const Vector3f* newValue )
{
	m_material->SetVector3( uniformNameVerbatim, newValue );
}


//--------------------------------------------------------------------------------------------------------------
void MeshRenderer::SetVector4( const std::string& uniformNameVerbatim, const Vector4f* newValue )
{
	m_material->SetVector4( uniformNameVerbatim, newValue );
}


//--------------------------------------------------------------------------------------------------------------
void MeshRenderer::SetMatrix4x4( const std::string& uniformNameVerbatim, bool shouldTranspose, const Matrix4x4f* newValue )
{
	m_material->SetMatrix4x4( uniformNameVerbatim, shouldTranspose, newValue );
}


//--------------------------------------------------------------------------------------------------------------
void MeshRenderer::SetColor( const std::string& uniformNameVerbatim, const Rgba* newValue )
{
	m_material->SetColor( uniformNameVerbatim, newValue );
}


//--------------------------------------------------------------------------------------------------------------
void MeshRenderer::SetSampler( const std::string& uniformNameVerbatim, unsigned int newSamplerID )
{
	m_material->SetSampler( uniformNameVerbatim, newSamplerID );
}


//--------------------------------------------------------------------------------------------------------------
void MeshRenderer::SetTexture( const std::string& uniformNameVerbatim, unsigned int newTextureID )
{
	m_material->SetTexture( uniformNameVerbatim, newTextureID );
}
