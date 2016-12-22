#pragma once

#include <map>
#include <vector>
#include "Engine/EngineCommon.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"


class VertexDefinition;


struct DrawInstruction //Compare notes.
{
	DrawInstruction()
		: m_type( VertexGroupingRule::AS_TRIANGLES )
		, m_startIndex( 0 )
		, m_count( 0 )
		, m_usingIndexBuffer( 0 )
	{
	}

	DrawInstruction( VertexGroupingRule type, unsigned int startIndex, unsigned int count, uint32_t useIndexBuffer )
		: m_type( type )
		, m_startIndex( startIndex )
		, m_count( count )
		, m_usingIndexBuffer( useIndexBuffer )
	{
	}

	VertexGroupingRule m_type; //i.e. interpreted by TheRenderer or MeshRenderer into a VertexGroupingRule.
	unsigned int m_startIndex; //e.g. render vertexes 2 to 4 with GL_LINES.
	unsigned int m_count; //how many vertexes from startIndex to include.
	uint32_t m_usingIndexBuffer; //Whether to use this logic in glDrawArrays or glDrawElements.
};


class Mesh //Has the VBO/IBOs, not uniforms. The "What" to render, but not the "How" (material) to render.
{
public:

	//Make blank, has no corresponding buffer.
	Mesh( const VertexDefinition& vertexDefinition ) 
		: m_vertexDefinition( &vertexDefinition )
		, m_vertices( nullptr )
		, m_indices( nullptr ) 
		, m_usingIndexBuffer( false )
	{
	}

	//Too complex a constructor to use Squirrel's CreateOrGet design pattern.
	Mesh( BufferUsage usage, const VertexDefinition& vertexDefinition, unsigned int numVertices, const void* vertexData, unsigned int numDrawInstructions, DrawInstruction* drawInstructions ); //No IBO.
	Mesh( BufferUsage usage, const VertexDefinition& vertexDefinition, unsigned int numVertices, const void* vertexData, unsigned int numIndices, void* indicesData, unsigned int numDrawInstructions, DrawInstruction* drawInstructions ); //With IBO.
//	Mesh( BufferUsage usage, const VertexDefinition& vertexDefinition, const Mesh& data );
	~Mesh();

	void SetThenUpdateMeshBuffers( unsigned int numVertices, void* vertexData ); //VBO only.
	void SetThenUpdateMeshBuffers( unsigned int numVertices, void* vertexData, unsigned int numIndices, void* indicesData ); //Can't have IBO without VBO.

	const VertexDefinition* GetVertexDefinition() const { return m_vertexDefinition; }
	unsigned int GetVertexBufferID() const { return ( m_vertices == nullptr ) ? NULL : m_vertices->GetBufferID(); };
	unsigned int GetIndexBufferID() const { return ( m_indices == nullptr ) ? NULL : m_indices->GetBufferID(); };
	size_t GetVertexBufferSize() const { return ( m_vertices == nullptr ) ? NULL : m_vertices->GetBufferSize(); };
	size_t GetIndexBufferSize() const { return ( m_indices == nullptr ) ? NULL : m_indices->GetBufferSize(); };

	const std::vector<DrawInstruction>& GetDrawInstructions() const { return m_drawInstructions; }
	void ClearDrawInstructions() { m_drawInstructions.clear(); }
	void AddDrawInstruction( const DrawInstruction& command ) { m_drawInstructions.push_back(command); }
	void AddDrawInstruction( VertexGroupingRule prim, unsigned int startIndex, unsigned int count, bool useIndexBuffer );
	void AddDrawInstructions( const std::vector< DrawInstruction >& instructions );

	bool UsesIndexBuffer() const { return m_usingIndexBuffer; }

private:

	TODO( "Abstract Vertex class" );
	TODO( "Make Mesh adhere to registry pattern keyed on name string like Material or ShaderProgram." );
	VertexBuffer* m_vertices;
	IndexBuffer* m_indices;

	const VertexDefinition* m_vertexDefinition;
	bool m_usingIndexBuffer; //Call glDrawElements instead of glDrawArrays in executing draw instructions.

	std::vector<DrawInstruction> m_drawInstructions;
};