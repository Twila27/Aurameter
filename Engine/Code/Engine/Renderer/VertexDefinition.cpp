#include "Engine/Renderer/VertexDefinition.hpp"


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>


//--------------------------------------------------------------------------------------------------------------
unsigned int GetOpenGLVertexFieldType( VertexFieldType engineVertexFieldType )
{
	switch ( engineVertexFieldType )
	{
	case VertexFieldType::VERTEX_FIELD_TYPE_FLOAT: return GL_FLOAT;
	case VertexFieldType::VERTEX_FIELD_TYPE_UNSIGNED_BYTE: return GL_UNSIGNED_BYTE;
	case VertexFieldType::VERTEX_FIELD_TYPE_UNSIGNED_INT: return GL_UNSIGNED_INT;
	default: ERROR_AND_DIE( "Unsupported VertexFieldType in GetOpenGLVertexFieldType in VertexAttribute.cpp!" );
	}
}


//--------------------------------------------------------------------------------------------------------------
size_t VertexAttribute::GetTypeSize( VertexFieldType type ) const
{
	switch ( type )
	{
	case VERTEX_FIELD_TYPE_FLOAT: return sizeof( float );
	case VERTEX_FIELD_TYPE_UNSIGNED_BYTE: return sizeof( unsigned char );
	case VERTEX_FIELD_TYPE_UNSIGNED_INT: return sizeof( unsigned int );
	default: ERROR_AND_DIE( "Unsupported VertexFieldType in VertexAttribute::GetTypeSize!" );
	}
}


//--------------------------------------------------------------------------------------------------------------
VertexDefinition::VertexDefinition( unsigned int vertexSize, unsigned int numAttributes, const VertexAttribute* attributes )
	: m_vertexSize( vertexSize )
	, m_numAttributes( numAttributes )
{
	for ( unsigned int attributeIndex = 0; attributeIndex < numAttributes; attributeIndex++ )
		m_vertexAttributes.push_back( attributes[ attributeIndex ] );
}


//--------------------------------------------------------------------------------------------------------------
const VertexAttribute* VertexDefinition::GetAttributeAtIndex( size_t index ) const
{
	ASSERT_OR_DIE( index < m_numAttributes, "GetAttribute Fail" ); 
	
	return &m_vertexAttributes[ index ];
}
