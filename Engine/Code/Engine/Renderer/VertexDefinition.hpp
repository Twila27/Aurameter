#pragma once
#include <string>
#include <vector>
#include "Engine/Error/ErrorWarningAssert.hpp"


enum VertexFieldType
{
	VERTEX_FIELD_TYPE_FLOAT,
	VERTEX_FIELD_TYPE_UNSIGNED_BYTE,
	VERTEX_FIELD_TYPE_UNSIGNED_INT
};
unsigned int GetOpenGLVertexFieldType( VertexFieldType engineVertexFieldType );


struct VertexAttribute //Defines VertexBuffer format, e.g. describing Vertex3D_PCT. Define one per Vertex class type.
{
	std::string m_attributeName;
//	unsigned int m_bindPoint; //No duplicates
	int m_count; //number of the following type, so matrix is a 4, GL_FLOAT4
	VertexFieldType m_fieldType; //e.g. GL_FLOAT, GL_UNSIGNED_BYTE.
	bool m_normalized; //Is this value normalized when it's sent in?
	unsigned int m_offset; //Offset into the vertex class with _offset macro.

	VertexAttribute() {}
	VertexAttribute( const std::string& attributeVerbatimName, int count, VertexFieldType fieldType, bool isNormalized, unsigned int offset )
		: m_attributeName( attributeVerbatimName )
		, m_count( count )
		, m_fieldType( fieldType )
		, m_normalized( isNormalized )
		, m_offset( offset ) 
	{
	}

	bool IsValid() const { return ( m_attributeName[ 0 ] != NULL ); }
	size_t GetTypeSize( VertexFieldType type ) const; //Returns e.g. sizeof( float ) for GL_FLOAT's equivalent VERTEX_FIELD_TYPE_FLOAT.
	size_t GetAttributeSize() const { return m_count * GetTypeSize( m_fieldType ); }
};


class VertexDefinition //Guarantee shader fields exist, match vertex, and match between a paired material and mesh.
{

public:
	VertexDefinition( unsigned int vertexSize, unsigned int numAttributes, const VertexAttribute* attributes );

	size_t GetVertexSize() const { return m_vertexSize; }
	const size_t GetNumAttributes() const { return m_numAttributes; }

	const VertexAttribute* GetAttributes() const { return m_vertexAttributes.data(); }
	const VertexAttribute* GetAttributeAtIndex( size_t index ) const;
	
// 	const VertexAttribute* FindAttribute( const std::string& name ) const;
// 	const VertexAttribute* FindAttribute( const unsigned int& bindPoint ) const;

	static const std::string VERTEX_ATTRIBUTE_NAME_POSITION;
	static const std::string VERTEX_ATTRIBUTE_NAME_COLOR;
	static const std::string VERTEX_ATTRIBUTE_NAME_UV0;
	static const std::string VERTEX_ATTRIBUTE_NAME_UV1;
	static const std::string VERTEX_ATTRIBUTE_NAME_UV2;
	static const std::string VERTEX_ATTRIBUTE_NAME_UV3;
	static const std::string VERTEX_ATTRIBUTE_NAME_TANGENT;
	static const std::string VERTEX_ATTRIBUTE_NAME_BITANGENT;
	static const std::string VERTEX_ATTRIBUTE_NAME_NORMAL;
	static const std::string VERTEX_ATTRIBUTE_NAME_SKINWEIGHTS;
	static const std::string VERTEX_ATTRIBUTE_NAME_JOINTINDICES;

private:
	unsigned int m_vertexSize;
	size_t m_numAttributes;
	std::vector<VertexAttribute> m_vertexAttributes; //A layout is an array of statements like the below, and one VertexAttribute gets defined in each Vertexes.hpp type struct as a class static constant:
};
