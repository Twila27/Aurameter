#include "Engine/Renderer/Mesh.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#include "Engine/Renderer/OpenGLExtensions.hpp"

#include "Engine/Renderer/VertexDefinition.hpp"


//--------------------------------------------------------------------------------------------------------------
Mesh::Mesh( BufferUsage usage, const VertexDefinition& vertexDefinition,
			unsigned int numVertices, const void* vertexData,
			unsigned int numDrawInstructions, DrawInstruction* drawInstructions )
	: m_vertices( new VertexBuffer( numVertices, vertexDefinition.GetVertexSize(), usage, vertexData ) )
	, m_vertexDefinition( &vertexDefinition )
	, m_usingIndexBuffer( false )
	, m_indices( nullptr )
{
	m_drawInstructions.reserve( numDrawInstructions );
	for ( unsigned int instructionIndex = 0; instructionIndex < numDrawInstructions; instructionIndex++ )
		m_drawInstructions.push_back( drawInstructions[ instructionIndex ] );
}


//--------------------------------------------------------------------------------------------------------------
Mesh::Mesh( BufferUsage usage, const VertexDefinition& vertexDefinition,
			unsigned int numVertices, const void* vertexData, 
			unsigned int numIndices, void* indicesData,
			unsigned int numDrawInstructions, DrawInstruction* drawInstructions )
	: Mesh( usage, vertexDefinition, numVertices, vertexData, numDrawInstructions, drawInstructions )
{
	m_indices = new IndexBuffer( numIndices, sizeof( unsigned int ), usage, indicesData );
	m_usingIndexBuffer = true;
}


//--------------------------------------------------------------------------------------------------------------
void Mesh::AddDrawInstruction( VertexGroupingRule prim, unsigned int startIndex, unsigned int count, bool useIndexBuffer )
{
	m_drawInstructions.push_back( DrawInstruction( prim, startIndex, count, useIndexBuffer ) );
}


//--------------------------------------------------------------------------------------------------------------
void Mesh::AddDrawInstructions( const std::vector< DrawInstruction >& instructions )
{
	for each ( DrawInstruction di in instructions )
		m_drawInstructions.push_back( di );
}


//--------------------------------------------------------------------------------------------------------------
void Mesh::SetThenUpdateMeshBuffers( unsigned int numVertices, void* vertexData )
{
	//NOTE THE SAME DRAW INSTRUCTIONS ARE ASSUMED!

//	m_usingIndexBuffer = false; //Commented out to allow VBO update while keeping same IBO, e.g. sprite renderer's mesh.

	ASSERT_OR_DIE( m_vertexDefinition != nullptr, "Mesh::SetThenUpdateMeshBuffers (No IBO) Missing VertexDefinition!" );
	size_t vertexSize = m_vertexDefinition->GetVertexSize();

	if ( m_vertices == nullptr )
		m_vertices = new VertexBuffer( numVertices, vertexSize, BufferUsage::STATIC_DRAW, vertexData );
	
	m_vertices->UpdateBuffer( vertexData, numVertices, vertexSize );
}


//--------------------------------------------------------------------------------------------------------------
void Mesh::SetThenUpdateMeshBuffers( unsigned int numVertices, void* vertexData, unsigned int numIndices, void* indicesData )
{
	m_usingIndexBuffer = ( numIndices > 0 );

	ASSERT_OR_DIE( m_vertexDefinition != nullptr, "Mesh::SetThenUpdateMeshBuffers (With IBO) Missing VertexDefinition!" );
	size_t vertexSize = m_vertexDefinition->GetVertexSize();

	if ( m_vertices == nullptr )
		m_vertices = new VertexBuffer( numVertices, vertexSize, BufferUsage::STATIC_DRAW, vertexData );

	
	if ( m_usingIndexBuffer && m_indices == nullptr )
		m_indices = new IndexBuffer( numIndices, sizeof( unsigned int ), BufferUsage::STATIC_DRAW, indicesData );

	m_vertices->UpdateBuffer( vertexData, numVertices, vertexSize ); 

	if ( m_usingIndexBuffer )
		 m_indices->UpdateBuffer( indicesData, numIndices, sizeof( unsigned int ) );
}


//--------------------------------------------------------------------------------------------------------------
Mesh::~Mesh()
{
	if ( m_usingIndexBuffer && m_indices != nullptr )
		delete m_indices;
	m_indices = nullptr;

	if ( m_vertices != nullptr )
		delete m_vertices;
}
