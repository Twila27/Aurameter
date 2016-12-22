#include "Engine/Renderer/Vertexes.hpp"


//--------------------------------------------------------------------------------------------------------------
/*STATIC*/ const std::string VertexDefinition::VERTEX_ATTRIBUTE_NAME_POSITION = "inPosition";
/*STATIC*/ const std::string VertexDefinition::VERTEX_ATTRIBUTE_NAME_COLOR = "inColor";
/*STATIC*/ const std::string VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV0 = "inUV0";
/*STATIC*/ const std::string VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV1 = "inUV1";
/*STATIC*/ const std::string VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV2 = "inUV2";
/*STATIC*/ const std::string VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV3 = "inUV3";
/*STATIC*/ const std::string VertexDefinition::VERTEX_ATTRIBUTE_NAME_TANGENT = "inTangent";
/*STATIC*/ const std::string VertexDefinition::VERTEX_ATTRIBUTE_NAME_BITANGENT = "inBitangent";
/*STATIC*/ const std::string VertexDefinition::VERTEX_ATTRIBUTE_NAME_NORMAL = "inNormal";
/*STATIC*/ const std::string VertexDefinition::VERTEX_ATTRIBUTE_NAME_SKINWEIGHTS = "inBoneWeights";
/*STATIC*/ const std::string VertexDefinition::VERTEX_ATTRIBUTE_NAME_JOINTINDICES = "inBoneIndices";
//Not defined in VertexDefinition to ensure initialization of statics occurs before the below.

//--------------------------------------------------------------------------------------------------------------
const VertexAttribute Vertex2D_PCT::ATTRIBUTES[] =
{
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_POSITION, 2, VERTEX_FIELD_TYPE_FLOAT, false, offsetof( Vertex2D_PCT, m_position ) ),
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_COLOR, 4, VERTEX_FIELD_TYPE_UNSIGNED_BYTE, true, offsetof( Vertex2D_PCT, m_color ) ),
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV0, 2, VERTEX_FIELD_TYPE_FLOAT, false, offsetof( Vertex2D_PCT, m_texCoords ) )
};
const VertexDefinition Vertex2D_PCT::DEFINITION = VertexDefinition( sizeof( Vertex2D_PCT ), 3, ATTRIBUTES ); //Must be below attribute definition.


//--------------------------------------------------------------------------------------------------------------
const VertexAttribute Vertex3D_PCT::ATTRIBUTES[] =
{
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_POSITION, 3, VERTEX_FIELD_TYPE_FLOAT, false, offsetof( Vertex3D_PCT, m_position ) ),
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_COLOR, 4, VERTEX_FIELD_TYPE_UNSIGNED_BYTE, true, offsetof( Vertex3D_PCT, m_color ) ),
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV0, 2, VERTEX_FIELD_TYPE_FLOAT, false, offsetof( Vertex3D_PCT, m_texCoords ) )
};
const VertexDefinition Vertex3D_PCT::DEFINITION = VertexDefinition( sizeof( Vertex3D_PCT ), 3, ATTRIBUTES ); //Must be below attribute definition.


//--------------------------------------------------------------------------------------------------------------
const VertexAttribute Vertex3D_PCUTB::ATTRIBUTES[] =
{
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_POSITION, 3, VERTEX_FIELD_TYPE_FLOAT, false, offsetof( Vertex3D_PCUTB, m_position ) ),
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_COLOR, 4, VERTEX_FIELD_TYPE_UNSIGNED_BYTE, true, offsetof( Vertex3D_PCUTB, m_color ) ),
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV0, 2, VERTEX_FIELD_TYPE_FLOAT, false, offsetof( Vertex3D_PCUTB, m_texCoords ) ),
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_TANGENT, 3, VERTEX_FIELD_TYPE_FLOAT, false, offsetof( Vertex3D_PCUTB, m_tangent ) ),
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_BITANGENT, 3, VERTEX_FIELD_TYPE_FLOAT, false, offsetof( Vertex3D_PCUTB, m_bitangent ) )
};
const VertexDefinition Vertex3D_PCUTB::DEFINITION = VertexDefinition( sizeof( Vertex3D_PCUTB ), 5, ATTRIBUTES ); //Must be below attribute definition.


//--------------------------------------------------------------------------------------------------------------
const VertexAttribute Vertex3D_PCTWI::ATTRIBUTES[] =
{
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_POSITION, 3, VERTEX_FIELD_TYPE_FLOAT, false, offsetof( Vertex3D_PCTWI, m_position ) ),
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_COLOR, 4, VERTEX_FIELD_TYPE_UNSIGNED_BYTE, true, offsetof( Vertex3D_PCTWI, m_color ) ),
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV0, 2, VERTEX_FIELD_TYPE_FLOAT, false, offsetof( Vertex3D_PCTWI, m_texCoords ) ),
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_SKINWEIGHTS, 4, VERTEX_FIELD_TYPE_FLOAT, false, offsetof( Vertex3D_PCTWI, m_boneWeights ) ),
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_JOINTINDICES, 4, VERTEX_FIELD_TYPE_UNSIGNED_INT, false, offsetof( Vertex3D_PCTWI, m_jointIndices ) )
};
const VertexDefinition Vertex3D_PCTWI::DEFINITION = VertexDefinition( sizeof( Vertex3D_PCTWI ), 5, ATTRIBUTES ); //Must be below attribute definition.
const Vector4f Vertex3D_PCTWI::DEFAULT_BONE_WEIGHTS = DEFAULT_BONE_WEIGHTS;
const Vector4<unsigned int> Vertex3D_PCTWI::DEFAULT_JOINT_INDICES = DEFAULT_JOINT_INDICES;


//--------------------------------------------------------------------------------------------------------------
const VertexAttribute Vertex3D_Superset::ATTRIBUTES[] =
{
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_POSITION, 3, VERTEX_FIELD_TYPE_FLOAT, false, offsetof( Vertex3D_Superset, m_position ) ),
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_COLOR, 4, VERTEX_FIELD_TYPE_UNSIGNED_BYTE, true, offsetof( Vertex3D_Superset, m_color ) ),
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV0, 2, VERTEX_FIELD_TYPE_FLOAT, false, offsetof( Vertex3D_Superset, m_texCoords0 ) ),
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV1, 2, VERTEX_FIELD_TYPE_FLOAT, false, offsetof( Vertex3D_Superset, m_texCoords1 ) ),
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV2, 2, VERTEX_FIELD_TYPE_FLOAT, false, offsetof( Vertex3D_Superset, m_texCoords2 ) ),
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV3, 2, VERTEX_FIELD_TYPE_FLOAT, false, offsetof( Vertex3D_Superset, m_texCoords3 ) ),
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_TANGENT, 3, VERTEX_FIELD_TYPE_FLOAT, false, offsetof( Vertex3D_Superset, m_tangent ) ),
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_BITANGENT, 3, VERTEX_FIELD_TYPE_FLOAT, false, offsetof( Vertex3D_Superset, m_bitangent ) ),
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_SKINWEIGHTS, 4, VERTEX_FIELD_TYPE_FLOAT, false, offsetof( Vertex3D_Superset, m_boneWeights ) ),
	VertexAttribute( VertexDefinition::VERTEX_ATTRIBUTE_NAME_JOINTINDICES, 4, VERTEX_FIELD_TYPE_UNSIGNED_INT, false, offsetof( Vertex3D_Superset, m_jointIndices ) )
};
const VertexDefinition Vertex3D_Superset::DEFINITION = VertexDefinition( sizeof( Vertex3D_Superset ), 10, ATTRIBUTES ); //Must be below attribute definition.
const Vector4f Vertex3D_Superset::DEFAULT_BONE_WEIGHTS = DEFAULT_BONE_WEIGHTS;
const Vector4<unsigned int> Vertex3D_Superset::DEFAULT_JOINT_INDICES = DEFAULT_JOINT_INDICES;


//--------------------------------------------------------------------------------------------------------------
Vertex2D_PCT::Vertex2D_PCT( const Vector2f& position, const Vector2f& texCoords /*= Vector2f::ZERO*/, const Rgba& color /*= Rgba::WHITE */ )
	: m_position( position )
	, m_color( color )
	, m_texCoords( texCoords )
{
}


//--------------------------------------------------------------------------------------------------------------
Vertex2D_PCT::Vertex2D_PCT( const Vector2f& position, const Rgba& color /*= Rgba::WHITE */, const Vector2f& texCoords /*= Vector2f::ZERO*/ )
	: m_position( position )
	, m_color( color )
	, m_texCoords( texCoords )
{
}


//--------------------------------------------------------------------------------------------------------------
Vertex2D_PCT::Vertex2D_PCT()
	: m_position()
	, m_color()
	, m_texCoords()
{
}


//--------------------------------------------------------------------------------------------------------------
Vertex3D_PCT::Vertex3D_PCT( const Vector3f& position, const Vector2f& texCoords /*= Vector2f::ZERO*/, const Rgba& color /*= Rgba::WHITE */ )
	: m_position( position )
	, m_color( color )
	, m_texCoords( texCoords )
{
}


//--------------------------------------------------------------------------------------------------------------
Vertex3D_PCT::Vertex3D_PCT( const Vector3f& position, const Rgba& color /*= Rgba::WHITE */, const Vector2f& texCoords /*= Vector2f::ZERO*/ )
	: m_position( position )
	, m_color( color )
	, m_texCoords( texCoords )
{
}


//--------------------------------------------------------------------------------------------------------------
Vertex3D_PCT::Vertex3D_PCT()
	: m_position()
	, m_color()
	, m_texCoords()
{
}


//--------------------------------------------------------------------------------------------------------------
Vertex3D_PCUTB::Vertex3D_PCUTB( const Vector3f& position, const Vector3f& tangent, const Vector3f& bitangent, 
								const Vector2f& texCoords /*= Vector2f::ZERO*/, const Rgba& color /*= Rgba::WHITE */ )
	: m_position( position )
	, m_color( color )
	, m_texCoords( texCoords )
	, m_tangent( tangent )
	, m_bitangent( bitangent )
{
}


//--------------------------------------------------------------------------------------------------------------
Vertex3D_PCUTB::Vertex3D_PCUTB( const Vector3f& position, const Vector3f& tangent, const Vector3f& bitangent, 
								const Rgba& color /*= Rgba::WHITE */, const Vector2f& texCoords /*= Vector2f::ZERO*/ )
	: m_position( position )
	, m_color( color )
	, m_texCoords( texCoords )
	, m_tangent( tangent )
	, m_bitangent( bitangent )
{
}

//--------------------------------------------------------------------------------------------------------------
Vertex3D_PCUTB::Vertex3D_PCUTB()
	: m_position()
	, m_color()
	, m_texCoords()
	, m_tangent()
	, m_bitangent()
{
}


//--------------------------------------------------------------------------------------------------------------
VertexCopyCallback* GetCopyFunctionForVertexDefinition( const VertexDefinition* vdefn )
{
	if ( nullptr == vdefn )
		return nullptr;

	if ( vdefn == &Vertex3D_PCT::DEFINITION )
		return CopyToVertex3D_PCT;

	if ( vdefn == &Vertex3D_PCUTB::DEFINITION )
		return CopyToVertex3D_PCUTB;

	if ( vdefn == &Vertex3D_Superset::DEFINITION )
		return CopyToVertex3D_Superset;

	ERROR_AND_DIE( "Request for Unsupported VertexCopyCallback!" );
}


//--------------------------------------------------------------------------------------------------------------
void CopyToVertex2D_PCT( void* dest, const Vertex3D_Superset& src )
{
	(void)( dest );
	(void)( src );

	throw new std::logic_error( "CopyToVertex2D_PCT Unimplemented!" );
}


//--------------------------------------------------------------------------------------------------------------
void CopyToVertex3D_PCT( void* dest, const Vertex3D_Superset& src )
{
	Vertex3D_PCT* vertex = (Vertex3D_PCT*)dest;
	vertex->m_position = src.m_position;
	vertex->m_color = src.m_color;
	vertex->m_texCoords = src.m_texCoords0;
}


//--------------------------------------------------------------------------------------------------------------
void CopyToVertex3D_PCUTB( void* dest, const Vertex3D_Superset& src )
{
	Vertex3D_PCUTB* vertex = (Vertex3D_PCUTB*)dest;
	vertex->m_position = src.m_position;
	vertex->m_tangent = src.m_tangent;
	vertex->m_bitangent = src.m_bitangent;
	vertex->m_color = src.m_color;
	vertex->m_texCoords = src.m_texCoords0;
}


//--------------------------------------------------------------------------------------------------------------
void CopyToVertex3D_Superset( void* dest, const Vertex3D_Superset& src )
{ 
	Vertex3D_Superset* vertex = (Vertex3D_Superset*)dest; 
	*vertex = src; 
}
