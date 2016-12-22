#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/Rgba.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Plane.hpp"
#include "Engine/FileUtils/Writers/BinaryWriter.hpp"
#include "Engine/FileUtils/Writers/FileBinaryWriter.hpp"
#include "Engine/FileUtils/Readers/BinaryReader.hpp"
#include "Engine/FileUtils/Readers/FileBinaryReader.hpp"
#include "Engine/Memory/BitUtils.hpp"


//--------------------------------------------------------------------------------------------------------------
MeshBuilder* g_lastLoadedMeshBuilder = nullptr;


//--------------------------------------------------------------------------------------------------------------
void MeshBuilder::Begin( VertexGroupingRule primitiveType, uint32_t usingIndexBuffer )
{
	m_currentStartIndex = m_currentVertices.size();
	m_currentVertexGroupingRule = primitiveType;
	m_usingIndexBuffer = usingIndexBuffer;
}


//--------------------------------------------------------------------------------------------------------------
void MeshBuilder::SetSkinWeights( const Vector4<unsigned int>& jointIndices, const Vector4f& boneWeights )
{
	m_currentTemplate.m_boneWeights = boneWeights;
	m_currentTemplate.m_jointIndices = jointIndices;

	//A vertex can't have more or less than a total of 100% weighted-ness.
	float totalWeight = m_currentTemplate.m_boneWeights.x 
		+ m_currentTemplate.m_boneWeights.y 
		+ m_currentTemplate.m_boneWeights.z 
		+ m_currentTemplate.m_boneWeights.w;
	if ( totalWeight != 0 )
	{
		m_currentTemplate.m_boneWeights.x /= totalWeight;
		m_currentTemplate.m_boneWeights.y /= totalWeight;
		m_currentTemplate.m_boneWeights.z /= totalWeight;
		m_currentTemplate.m_boneWeights.w /= totalWeight;
	}
	else
		m_currentTemplate.m_boneWeights = Vertex3D_Superset::DEFAULT_BONE_WEIGHTS;

	SetVertexDataMaskBit( MESH_VERTEX_ATTRIBUTE_SKINWEIGHTS );
}


//--------------------------------------------------------------------------------------------------------------
void MeshBuilder::ClearBoneWeights()
{
	m_currentTemplate.m_boneWeights = Vertex3D_Superset::DEFAULT_BONE_WEIGHTS;
	m_currentTemplate.m_jointIndices = Vertex3D_Superset::DEFAULT_JOINT_INDICES;
	UnsetVertexDataMaskBit( MESH_VERTEX_ATTRIBUTE_SKINWEIGHTS );
}

//--------------------------------------------------------------------------------------------------------------
void MeshBuilder::End()
{
	unsigned int count = ( ( m_usingIndexBuffer == 1 ) ? m_currentIndices.size() : m_currentVertices.size() ) - m_currentStartIndex;

	m_currentInstructions.push_back( DrawInstruction(
		m_currentVertexGroupingRule,
		m_currentStartIndex,
		count,
		m_usingIndexBuffer
	) );
}


//--------------------------------------------------------------------------------------------------------------
void MeshBuilder::SetVertexDataMaskBit( MeshVertexAttributeBitIndex bitTag )
{
	m_currentVertexDataMask |= GET_BIT_AT_BITFIELD_INDEX( bitTag );
}


//--------------------------------------------------------------------------------------------------------------
void MeshBuilder::UnsetVertexDataMaskBit( MeshVertexAttributeBitIndex bitTag )
{
	m_currentVertexDataMask &= ~GET_BIT_AT_BITFIELD_INDEX( bitTag );
}


//--------------------------------------------------------------------------------------------------------------
void MeshBuilder::CopyToMesh( Mesh* mesh )
{
	VertexCopyCallback* copyFromVertexSupersetToSubset = GetCopyFunctionForVertexDefinition( mesh->GetVertexDefinition() );
	size_t vertexSize = mesh->GetVertexDefinition()->GetVertexSize();
	size_t vertexCount = m_currentVertices.size();

	//This callback pattern is an alternative to a giant switch(vertexDefinition).
	size_t totalVertexSubsetBufferSize = vertexSize * vertexCount;
	byte_t* vertexSubsetBuffer = new byte_t[ totalVertexSubsetBufferSize ]; //Have to do it thus since mesh's vdefn != VertexSuperset.

	//Can't just push back to mesh VBO--we don't know anything about its size.
	for ( size_t vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++ )
	{
		size_t startOfNextVertex = vertexIndex * vertexSize;
		copyFromVertexSupersetToSubset( vertexSubsetBuffer + startOfNextVertex, m_currentVertices[ vertexIndex ] ); //Pointer arithmetic.
	}

	mesh->SetThenUpdateMeshBuffers( vertexCount, vertexSubsetBuffer, m_currentIndices.size(), m_currentIndices.data() );

	mesh->ClearDrawInstructions();
	mesh->AddDrawInstructions( m_currentInstructions );
}


//--------------------------------------------------------------------------------------------------------------
void MeshBuilder::BuildTriangle( const Vector3f& topLeft, const Vector3f& bottomLeft, const Vector3f& bottomRight )
{
	Vector3f tangent = bottomRight - bottomLeft;
	Vector3f bitangent = topLeft - bottomLeft;
	Vector3f normal = CrossProduct( tangent, bitangent ); //tangent-to-bitangent for RHS.
	bitangent = CrossProduct( normal, tangent ); //normal-to-tangent for RHS.

	//Note v coordinates may need inversion.
	Begin( VertexGroupingRule::AS_TRIANGLES, false ); //IBO won't save anything on a triangle.
	{
		SetTBN( tangent, bitangent, normal );

		SetUV0( Vector2f( 0.f, 1.f ) );
		AddVertex( bottomLeft );

		SetUV0( Vector2f( 1.f, 1.f ) );
		AddVertex( bottomRight );

		SetUV0( Vector2f( 0.f, 0.f ) );
		AddVertex( topLeft );
	}
	End();
}


//--------------------------------------------------------------------------------------------------------------
void MeshBuilder::BuildPlane( const Vector3f& initialPosition, const Vector3f& rightPlanarDirection, const Vector3f& upPlanarDirection,
							  float startX, float endX, unsigned int xSectionCount,
							  float startY, float endY, unsigned int ySectionCount )
{
	Plane plane;
	plane.initialPosition = initialPosition;
	plane.rightPlanarDirection = rightPlanarDirection;
	plane.upPlanarDirection = upPlanarDirection;

	BuildSurfacePatch( GetPositionOnPlane, &plane, startX, endX, xSectionCount, startY, endY, ySectionCount );
}


//--------------------------------------------------------------------------------------------------------------
void MeshBuilder::BuildSurfacePatch( Vector3f( *surfaceEquation )( const Vector2f& position, const void* surfaceObject ), void* userData, 
									 float startX, float endX, unsigned int xSectionCount, 
									 float startY, float endY, unsigned int ySectionCount )
{
	ASSERT_OR_DIE( xSectionCount > 0, "xSectionCount <= 0 in MeshBuilder::BuildSurfacePatch!" );
	ASSERT_OR_DIE( ySectionCount > 0, "ySectionCount <= 0 in MeshBuilder::BuildSurfacePatch!" );

	//Note v coordinates may need inversion.
	Begin( VertexGroupingRule::AS_TRIANGLES, true );
	{
		unsigned int xVertexCount = xSectionCount + 1;
		unsigned int yVertexCount = ySectionCount + 1;

		const float xRange = endX - startX; //If endX < startX, renders backward!
		const float yRange = endY - startY;

		float oneOverSectionCountOnX = 1.f / (float)xSectionCount;
		float oneOverSectionCountOnY = 1.f / (float)ySectionCount;

		const float xStep = xRange * oneOverSectionCountOnX;
		const float yStep = yRange * oneOverSectionCountOnY;

		const float uStep = oneOverSectionCountOnX; //1.f being u's max.
		const float vStep = oneOverSectionCountOnY; //1.f being v's max.

		float x, y( startY );
		float u, v( 0.f );

		unsigned int startingVertexIndex = m_currentVertices.size();
		const float deltaStep = .001f;

		//Generate All Vertices
		for ( unsigned int yIndex = 0; yIndex < yVertexCount; yIndex++ )
		{
			x = startX;
			u = 0.f;

			for ( unsigned int xIndex = 0; xIndex < xVertexCount; xIndex++ )
			{
				SetUV0( Vector2f( u, v ) );

				Vector3f tangent = surfaceEquation( Vector2f( x + deltaStep, y ), userData ) - surfaceEquation( Vector2f( x - deltaStep, y ), userData );
				Vector3f bitangent = surfaceEquation( Vector2f( x, y + deltaStep ), userData ) - surfaceEquation( Vector2f( x, y - deltaStep ), userData );

				tangent.Normalize();
				bitangent.Normalize();

				Vector3f normal = CrossProduct( tangent, bitangent ); //tangent-to-bitangent for RHS.
				bitangent = CrossProduct( normal, tangent ); //normal-to-tangent for RHS.

				SetTBN( tangent, bitangent, normal );

				Vector3f position = surfaceEquation( Vector2f( x, y ), userData );

				AddVertex( position );
				x += xStep;
				u += uStep;
			}

			y += yStep;
			v += vStep;
		}

		//Add Indices to generate our surface patch's current quad.
		for ( unsigned int yIndex = 0; yIndex < ySectionCount; yIndex++ )
		{
			for ( unsigned int xIndex = 0; xIndex < xSectionCount; xIndex++ )
			{
				unsigned int bottomLeftIndex = startingVertexIndex + ( yIndex * xVertexCount ) + xIndex;
				unsigned int bottomRightIndex = bottomLeftIndex + 1;

				unsigned int topLeftIndex = bottomLeftIndex + xVertexCount;
				unsigned int topRightIndex = topLeftIndex + 1;

				AddIndicesForQuad( topLeftIndex, topRightIndex, bottomLeftIndex, bottomRightIndex );
			}
		}
	}
	End();
}


//--------------------------------------------------------------------------------------------------------------
void MeshBuilder::AddIndicesForQuad( unsigned int topLeftIndex, unsigned int topRightIndex, unsigned int bottomLeftIndex, unsigned int bottomRightIndex )
{
	//Clockwise: 0, 1, 2, 0, 2, 3.
	//3 0
	//2 1
	m_currentIndices.push_back( topRightIndex );
	m_currentIndices.push_back( bottomRightIndex );
	m_currentIndices.push_back( bottomLeftIndex );

	m_currentIndices.push_back( topRightIndex );
	m_currentIndices.push_back( bottomLeftIndex );
	m_currentIndices.push_back( topLeftIndex );
}


//--------------------------------------------------------------------------------------------------------------
unsigned int MeshBuilder::AppendMeshBuilderWhileMasksAndMaterialsMatch( const std::vector< MeshBuilder* >& others )
{
	unsigned int numMeshBuildersAppended;

	for ( numMeshBuildersAppended = 0; numMeshBuildersAppended < others.size(); numMeshBuildersAppended++ )
	{
		if ( this == others[ numMeshBuildersAppended ] )
			continue;

		if ( AppendMeshBuilderIfMasksMatch( *others[ numMeshBuildersAppended ] ) == false )
			break;
	}

	return numMeshBuildersAppended;
}


//--------------------------------------------------------------------------------------------------------------
bool MeshBuilder::AppendMeshBuilderIfMasksMatch( const MeshBuilder& other )
{
	//If this has an empty data mask, clone the other's.
	if ( m_currentVertexDataMask == 0 )
		m_currentVertexDataMask = other.m_currentVertexDataMask;

	if ( m_currentVertexDataMask != other.m_currentVertexDataMask )
		return false;

	if ( m_currentMaterialID != other.m_currentMaterialID )
		return false;

	//Store off how many vertices and indices *this currently has, to offset the argument's draw instructions later.
	unsigned int firstBuilderLastVBOIndex = m_currentVertices.size();
	unsigned int firstBuilderLastIBOIndex = m_currentIndices.size();

	//1. Combine VBOs and IBOs back to back.
	for ( unsigned int vertexIndex = 0; vertexIndex < other.m_currentVertices.size(); vertexIndex++ )
	{
		m_currentVertices.push_back( other.m_currentVertices[ vertexIndex ] );
	}
	if ( other.m_usingIndexBuffer )
	{
		for ( unsigned int indicesIndex = 0; indicesIndex < other.m_currentIndices.size(); indicesIndex++ )
		{
			m_currentIndices.push_back( other.m_currentIndices[ indicesIndex ] );
		}
	}

	//2. Offset all draw instructions of _other by the last index / count / current vertex of *this.
	for ( unsigned int instructionIndex = 0; instructionIndex < other.m_currentInstructions.size(); instructionIndex++ )
	{
		const DrawInstruction& source = other.m_currentInstructions[ instructionIndex ];
		if ( source.m_usingIndexBuffer )
		{
			m_currentInstructions.push_back( DrawInstruction(
				source.m_type,
				source.m_startIndex + firstBuilderLastIBOIndex,
				source.m_count,
				source.m_usingIndexBuffer
			) );
		}
		else
		{
			m_currentInstructions.push_back( DrawInstruction(
				source.m_type,
				source.m_startIndex + firstBuilderLastVBOIndex,
				source.m_count,
				source.m_usingIndexBuffer
			) );
		}
	}

	TODO( "Have 2 draw instructions for drawing triangles become 1 instruction with twice the reach." );
	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool MeshBuilder::WriteToFile( const char* filename, bool appendToFile, int endianMode )
{
	bool didWrite = false;

	FileBinaryWriter writer;
	writer.SetEndianMode( (EndianMode)endianMode );

	didWrite = writer.open( filename, appendToFile );
	if ( !didWrite )
	{
		DebuggerPrintf( "File not accessible in MeshBuilder::WriteToFile!" );
		return false;
	}

	didWrite = this->WriteToStream( writer );
	if ( !didWrite )
	{
		DebuggerPrintf( "WriteToStream failed in MeshBuilder::WriteToFile!" );
		return false;
	}
	writer.close();

	return didWrite;
}


//--------------------------------------------------------------------------------------------------------------
bool MeshBuilder::WriteToStream( BinaryWriter& writer ) //Can have a bunch of meshes written to one file!
{
	//See bottom of .hpp for format.
	bool didWrite = false;
	
	//BE EXPLICIT ABOUT SIZE. size_t differs 32-bit versus 64-bit.
	unsigned int verticesCount = m_currentVertices.size();
	unsigned int indicesCount = m_currentIndices.size();
	unsigned int instructionsCount = m_currentInstructions.size();

	didWrite = writer.Write<uint32_t>( s_FILE_VERSION ); 
	didWrite = writer.WriteString( m_currentMaterialID.c_str() );
	didWrite = writer.Write<uint32_t>( verticesCount );
	didWrite = writer.Write<uint32_t>( indicesCount );
	didWrite = writer.Write<uint32_t>( instructionsCount );

	ASSERT_OR_DIE( m_currentVertexDataMask != 0, "MeshBuilder's DataMask not set!" );
	didWrite = WriteVertexDataMask( writer, m_currentVertexDataMask );
	didWrite = WriteVertices( writer );
	didWrite = WriteIndices( writer );
	didWrite = WriteDrawInstructions( writer );

	return didWrite;
}


//--------------------------------------------------------------------------------------------------------------
bool MeshBuilder::WriteVertices( BinaryWriter& writer )
{
	bool didWrite = false;

	//The problem: ( writer.WriteBytes( m_currentVertices.data(), verticesCount ) == verticesCount );
		//Only writes # bytes == # vertices, but sizeof a vertex != 1 byte.
		//Furthermore, for Endian mode conversion, need to write member by member, not struct by struct.

	//Loop bounds just used so we have some point at which we know to exit.
	for ( unsigned int attributeIndex = 0; attributeIndex < Vertex3D_Superset::DEFINITION.GetNumAttributes(); attributeIndex++ )
	{
		switch ( attributeIndex )
		{
		case MESH_VERTEX_ATTRIBUTE_POSITION:
			if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( m_currentVertexDataMask, MESH_VERTEX_ATTRIBUTE_POSITION ) != 0 )
			{
				//By writing per-member, supports Endian conversions.
				for ( const Vertex3D_Superset& vertex : m_currentVertices )
				{
					didWrite = writer.Write<float>( vertex.m_position.x );
					didWrite = writer.Write<float>( vertex.m_position.y );
					didWrite = writer.Write<float>( vertex.m_position.z );
				}
			}
			break;
		case MESH_VERTEX_ATTRIBUTE_COLOR:
			if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( m_currentVertexDataMask, MESH_VERTEX_ATTRIBUTE_COLOR ) != 0 )
			{
				//By writing per-member, supports Endian conversions.
				for ( const Vertex3D_Superset& vertex : m_currentVertices )
				{
					didWrite = writer.Write<byte_t>( vertex.m_color.red );
					didWrite = writer.Write<byte_t>( vertex.m_color.green );
					didWrite = writer.Write<byte_t>( vertex.m_color.blue );
					didWrite = writer.Write<byte_t>( vertex.m_color.alphaOpacity );
				}
			}
			break;
		case MESH_VERTEX_ATTRIBUTE_UV0:
			if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( m_currentVertexDataMask, MESH_VERTEX_ATTRIBUTE_UV0 ) != 0 )
			{
				//By writing per-member, supports Endian conversions.
				for ( const Vertex3D_Superset& vertex : m_currentVertices )
				{
					didWrite = writer.Write<float>( vertex.m_texCoords0.x );
					didWrite = writer.Write<float>( vertex.m_texCoords0.y );
				}
			}
			break;
		case MESH_VERTEX_ATTRIBUTE_UV1:
			if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( m_currentVertexDataMask, MESH_VERTEX_ATTRIBUTE_UV1 ) != 0 )
			{
				//By writing per-member, supports Endian conversions.
				for ( const Vertex3D_Superset& vertex : m_currentVertices )
				{
					didWrite = writer.Write<float>( vertex.m_texCoords1.x );
					didWrite = writer.Write<float>( vertex.m_texCoords1.y );
				}
			}
			break;
		case MESH_VERTEX_ATTRIBUTE_UV2:
			if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( m_currentVertexDataMask, MESH_VERTEX_ATTRIBUTE_UV2 ) != 0 )
			{
				//By writing per-member, supports Endian conversions.
				for ( const Vertex3D_Superset& vertex : m_currentVertices )
				{
					didWrite = writer.Write<float>( vertex.m_texCoords2.x );
					didWrite = writer.Write<float>( vertex.m_texCoords2.y );
				}
			}
			break;
		case MESH_VERTEX_ATTRIBUTE_UV3:
			if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( m_currentVertexDataMask, MESH_VERTEX_ATTRIBUTE_UV3 ) != 0 )
			{
				//By writing per-member, supports Endian conversions.
				for ( const Vertex3D_Superset& vertex : m_currentVertices )
				{
					didWrite = writer.Write<float>( vertex.m_texCoords3.x );
					didWrite = writer.Write<float>( vertex.m_texCoords3.y );
				}
			}
			break;
		case MESH_VERTEX_ATTRIBUTE_TANGENT:
			if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( m_currentVertexDataMask, MESH_VERTEX_ATTRIBUTE_TANGENT ) != 0 )
			{
				//By writing per-member, supports Endian conversions.
				for ( const Vertex3D_Superset& vertex : m_currentVertices )
				{
					didWrite = writer.Write<float>( vertex.m_tangent.y );
					didWrite = writer.Write<float>( vertex.m_tangent.x );
					didWrite = writer.Write<float>( vertex.m_tangent.z );
				}
			}
			break;
		case MESH_VERTEX_ATTRIBUTE_BITANGENT:
			if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( m_currentVertexDataMask, MESH_VERTEX_ATTRIBUTE_BITANGENT ) != 0 )
			{
				//By writing per-member, supports Endian conversions.
				for ( const Vertex3D_Superset& vertex : m_currentVertices )
				{
					didWrite = writer.Write<float>( vertex.m_bitangent.x );
					didWrite = writer.Write<float>( vertex.m_bitangent.y );
					didWrite = writer.Write<float>( vertex.m_bitangent.z );
				}
			}
			break;
		case MESH_VERTEX_ATTRIBUTE_NORMAL:
			if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( m_currentVertexDataMask, MESH_VERTEX_ATTRIBUTE_NORMAL ) != 0 )
			{
				//By writing per-member, supports Endian conversions.
				for ( const Vertex3D_Superset& vertex : m_currentVertices )
				{
					didWrite = writer.Write<float>( vertex.m_normal.x );
					didWrite = writer.Write<float>( vertex.m_normal.y );
					didWrite = writer.Write<float>( vertex.m_normal.z );
				}
			}
			break;
		case MESH_VERTEX_ATTRIBUTE_SKINWEIGHTS:
			if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( m_currentVertexDataMask, MESH_VERTEX_ATTRIBUTE_SKINWEIGHTS ) != 0 )
			{
				//By writing per-member, supports Endian conversions.
				for ( Vertex3D_Superset& vertex : m_currentVertices )
				{
					didWrite = writer.Write<uint32_t>( vertex.m_jointIndices.x );
					didWrite = writer.Write<uint32_t>( vertex.m_jointIndices.y );
					didWrite = writer.Write<uint32_t>( vertex.m_jointIndices.z );
					didWrite = writer.Write<uint32_t>( vertex.m_jointIndices.w );

					didWrite = writer.Write<float>( vertex.m_boneWeights.x );
					didWrite = writer.Write<float>( vertex.m_boneWeights.y );
					didWrite = writer.Write<float>( vertex.m_boneWeights.z );
					didWrite = writer.Write<float>( vertex.m_boneWeights.w );
				}
			}
		}
		
	}

	return didWrite;
}


//--------------------------------------------------------------------------------------------------------------
bool MeshBuilder::WriteIndices( BinaryWriter& writer )
{
	bool didWrite = false;

	//The problem: ( writer.WriteBytes( m_currentIndices.data(), indicesCount ) == indicesCount );
	//Only reads # bytes == # indices, but sizeof(uint) != 1 byte.
	//Furthermore, for Endian mode conversion, need to read member by member, not struct by struct.

	for ( unsigned int index : m_currentIndices )
		didWrite = writer.Write<uint32_t>( index );

	return didWrite;
}


//--------------------------------------------------------------------------------------------------------------
bool MeshBuilder::WriteDrawInstructions( BinaryWriter& writer )
{	
	bool didWrite = false; 

	//The problem: ( writer.WriteBytes( m_currentInstructions.data(), instructionCount ) == instructionCount );
		//Only writes # bytes == # instructions, but sizeof an instruction != 1 byte.
		//Furthermore, for Endian mode conversion, need to write member by member, not struct by struct.

	for ( const DrawInstruction& instruction : m_currentInstructions )
	{
		didWrite = writer.Write<uint32_t>( instruction.m_type );
		didWrite = writer.Write<uint32_t>( instruction.m_startIndex );
		didWrite = writer.Write<uint32_t>( instruction.m_count );
		didWrite = writer.Write<uint32_t>( instruction.m_usingIndexBuffer );
	}

	return didWrite;
}

//--------------------------------------------------------------------------------------------------------------
bool MeshBuilder::WriteVertexDataMask( BinaryWriter& writer, uint32_t vertexDataMask )
{
	bool didWrite = false;

	if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( vertexDataMask, MESH_VERTEX_ATTRIBUTE_POSITION ) != 0 )
		didWrite = writer.WriteString( VertexDefinition::VERTEX_ATTRIBUTE_NAME_POSITION.c_str() ); //Note this will write a \0 onto the end, important for ReadDataMask/ReadString.

	if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( vertexDataMask, MESH_VERTEX_ATTRIBUTE_COLOR ) != 0 )
		didWrite = writer.WriteString( VertexDefinition::VERTEX_ATTRIBUTE_NAME_COLOR.c_str() );

	if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( vertexDataMask, MESH_VERTEX_ATTRIBUTE_UV0 ) != 0 )
		didWrite = writer.WriteString( VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV0.c_str() );

	if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( vertexDataMask, MESH_VERTEX_ATTRIBUTE_UV1 ) != 0 )
		didWrite = writer.WriteString( VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV1.c_str() );

	if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( vertexDataMask, MESH_VERTEX_ATTRIBUTE_UV2 ) != 0 )
		didWrite = writer.WriteString( VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV2.c_str() );

	if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( vertexDataMask, MESH_VERTEX_ATTRIBUTE_UV3 ) != 0 )
		didWrite = writer.WriteString( VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV3.c_str() );

	if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( vertexDataMask, MESH_VERTEX_ATTRIBUTE_TANGENT ) != 0 )
		didWrite = writer.WriteString( VertexDefinition::VERTEX_ATTRIBUTE_NAME_TANGENT.c_str() );

	if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( vertexDataMask, MESH_VERTEX_ATTRIBUTE_BITANGENT ) != 0 )
		didWrite = writer.WriteString( VertexDefinition::VERTEX_ATTRIBUTE_NAME_BITANGENT.c_str() );

	if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( vertexDataMask, MESH_VERTEX_ATTRIBUTE_NORMAL ) != 0 )
		didWrite = writer.WriteString( VertexDefinition::VERTEX_ATTRIBUTE_NAME_NORMAL.c_str() );

	if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( vertexDataMask, MESH_VERTEX_ATTRIBUTE_SKINWEIGHTS ) != 0 )
		didWrite = writer.WriteString( VertexDefinition::VERTEX_ATTRIBUTE_NAME_SKINWEIGHTS.c_str() ); 
	
	//Note this will write a \0 onto the end, important for ReadDataMask/ReadString:
	didWrite = writer.WriteString( nullptr ); //See ReadDataMask, used to end loop.

	return didWrite;
}


//--------------------------------------------------------------------------------------------------------------
bool MeshBuilder::IsSkinned() const
{
	return m_currentTemplate.m_boneWeights != Vector4f( 0.f );
		//Doesn't sum to 1, meaning it was never set.
}


//--------------------------------------------------------------------------------------------------------------
const VertexDefinition* MeshBuilder::GetVertexDefinitionFromVertexDataMask()
{
	FIXME( "Test whether we can stop using only Superset for all vertex data masks!" );
	return &Vertex3D_Superset::DEFINITION;

//	if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( m_currentVertexDataMask, MESH_VERTEX_ATTRIBUTE_UV3 ) != 0 )
//		return &Vertex3D_Superset::DEFINITION;
//
//	else if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( m_currentVertexDataMask, MESH_VERTEX_ATTRIBUTE_TANGENT ) != 0 )
//		return &Vertex3D_PCUTB::DEFINITION;
//
//	else if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( m_currentVertexDataMask, MESH_VERTEX_ATTRIBUTE_POSITION ) != 0 )
//		return &Vertex3D_PCT::DEFINITION;
//
//	else ERROR_AND_DIE( "MeshBuilder::GetVertexDefinitionFromVertexDataMask Found Unsupported Vertex Format!" );
}


//--------------------------------------------------------------------------------------------------------------
void MeshBuilder::SetVertexDataMaskFromVertexDefinition( const VertexDefinition* vertexDefinition )
{
	m_currentVertexDataMask = 0;

	for ( unsigned int attributeIndex = 0; attributeIndex < vertexDefinition->GetNumAttributes(); attributeIndex++ )
	{
		const std::string& attributeName = vertexDefinition->GetAttributeAtIndex( attributeIndex )->m_attributeName;

		if ( attributeName == VertexDefinition::VERTEX_ATTRIBUTE_NAME_POSITION )
			m_currentVertexDataMask |= GET_BIT_AT_BITFIELD_INDEX( MESH_VERTEX_ATTRIBUTE_POSITION );

		else if ( attributeName == VertexDefinition::VERTEX_ATTRIBUTE_NAME_COLOR )
			m_currentVertexDataMask |= GET_BIT_AT_BITFIELD_INDEX( MESH_VERTEX_ATTRIBUTE_COLOR );

		else if ( attributeName == VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV0 )
			m_currentVertexDataMask |= GET_BIT_AT_BITFIELD_INDEX( MESH_VERTEX_ATTRIBUTE_UV0 );

		else if ( attributeName == VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV1 )
			m_currentVertexDataMask |= GET_BIT_AT_BITFIELD_INDEX( MESH_VERTEX_ATTRIBUTE_UV1 );

		else if ( attributeName == VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV2 )
			m_currentVertexDataMask |= GET_BIT_AT_BITFIELD_INDEX( MESH_VERTEX_ATTRIBUTE_UV2 );

		else if ( attributeName == VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV3 )
			m_currentVertexDataMask |= GET_BIT_AT_BITFIELD_INDEX( MESH_VERTEX_ATTRIBUTE_UV3 );

		else if ( attributeName == VertexDefinition::VERTEX_ATTRIBUTE_NAME_TANGENT )
			m_currentVertexDataMask |= GET_BIT_AT_BITFIELD_INDEX( MESH_VERTEX_ATTRIBUTE_TANGENT );

		else if ( attributeName == VertexDefinition::VERTEX_ATTRIBUTE_NAME_BITANGENT )
			m_currentVertexDataMask |= GET_BIT_AT_BITFIELD_INDEX( MESH_VERTEX_ATTRIBUTE_BITANGENT );

		else if ( attributeName == VertexDefinition::VERTEX_ATTRIBUTE_NAME_NORMAL )
			m_currentVertexDataMask |= GET_BIT_AT_BITFIELD_INDEX( MESH_VERTEX_ATTRIBUTE_NORMAL );

		else
			ERROR_AND_DIE( "Unsupported Attribute Name in MeshBuilder::SetVertexDataMaskFromVertexDefinition!" );
	}
}

//--------------------------------------------------------------------------------------------------------------
bool MeshBuilder::ReadFromFile( const char* filename, int endianMode )
{
	bool didRead = false;

	FileBinaryReader reader;
	reader.SetEndianMode( (EndianMode)endianMode );

	didRead = reader.open( filename );
	if ( !didRead )
	{
		DebuggerPrintf( "File not found in MeshBuilder::ReadFromFile!" );
		return false;
	}

	didRead = this->ReadFromStream( reader );
	if ( !didRead )
	{
		DebuggerPrintf( "ReadFromStream failed in MeshBuilder::ReadFromFile!" );
		return false;
	}
	reader.close();

	return didRead;
}


//--------------------------------------------------------------------------------------------------------------
bool MeshBuilder::ReadFromStream( BinaryReader& reader )
{
	//See bottom of .hpp for format.
	bool didRead = false;

	//BE EXPLICIT ABOUT SIZE. size_t differs 32-bit versus 64-bit.
	unsigned int verticesCount = 0;
	unsigned int indicesCount = 0;
	unsigned int instructionsCount = 0;

	uint32_t fileVersion;
	didRead = reader.Read<uint32_t>( &fileVersion );

	DebuggerPrintf( "Reading FileVersion %u vs. Current FileVersion %u", fileVersion, s_FILE_VERSION );

	didRead = reader.ReadString( m_currentMaterialID );
	didRead = reader.Read<uint32_t>( &verticesCount ); //!\ If Endian mode is wrong, a small count becomes gigantic!
	didRead = reader.Read<uint32_t>( &indicesCount );
	didRead = reader.Read<uint32_t>( &instructionsCount );

	ASSERT_OR_DIE( m_currentVertexDataMask == 0, "MeshBuilder's DataMask not set!" );
	m_currentVertexDataMask = ReadVertexDataMask( reader );

	//!\ WILL CRASH HERE IF ENDIAN MODE IS INCORRECT, BECAUSE VERTICES WILL RESIZE TO A GIGANTIC VERTEX COUNT 
	m_currentVertices.resize( verticesCount );
	didRead = ReadVertices( reader );

	m_currentIndices.resize( indicesCount );
	didRead = ReadIndices( reader ); 

	m_currentInstructions.resize( instructionsCount );
	didRead = ReadDrawInstructions( reader );

	return didRead;
}


//--------------------------------------------------------------------------------------------------------------
bool MeshBuilder::ReadVertices( BinaryReader& reader )
{
	bool didRead = false;

	//The problem: ( writer.WriteBytes( m_currentVertices.data(), verticesCount ) == verticesCount );
	//Only reads # bytes == # vertices, but sizeof a vertex != 1 byte.
	//Furthermore, for Endian mode conversion, need to read member by member, not struct by struct.

	//Loop bounds just used so we have some point at which we know to exit.
	for ( unsigned int attributeIndex = 0; attributeIndex < Vertex3D_Superset::DEFINITION.GetNumAttributes(); attributeIndex++ )
	{
		switch ( attributeIndex )
		{
		case MESH_VERTEX_ATTRIBUTE_POSITION:
			if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( m_currentVertexDataMask, MESH_VERTEX_ATTRIBUTE_POSITION ) != 0 )
			{
				//By reading per-member, supports Endian conversions.
				for ( Vertex3D_Superset& vertex : m_currentVertices )
				{
					didRead = reader.Read<float>( &vertex.m_position.x );
					didRead = reader.Read<float>( &vertex.m_position.y );
					didRead = reader.Read<float>( &vertex.m_position.z );
				}
			}
			break;
		case MESH_VERTEX_ATTRIBUTE_COLOR:
			if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( m_currentVertexDataMask, MESH_VERTEX_ATTRIBUTE_COLOR ) != 0 )
			{
				//By reading per-member, supports Endian conversions.
				for ( Vertex3D_Superset& vertex : m_currentVertices )
				{
					didRead = reader.Read<byte_t>( &vertex.m_color.red );
					didRead = reader.Read<byte_t>( &vertex.m_color.green );
					didRead = reader.Read<byte_t>( &vertex.m_color.blue );
					didRead = reader.Read<byte_t>( &vertex.m_color.alphaOpacity );
				}
			}
			break;
		case MESH_VERTEX_ATTRIBUTE_UV0:
			if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( m_currentVertexDataMask, MESH_VERTEX_ATTRIBUTE_UV0 ) != 0 )
			{
				//By reading per-member, supports Endian conversions.
				for ( Vertex3D_Superset& vertex : m_currentVertices )
				{
					didRead = reader.Read<float>( &vertex.m_texCoords0.x );
					didRead = reader.Read<float>( &vertex.m_texCoords0.y );
				}
			}
			break;
		case MESH_VERTEX_ATTRIBUTE_UV1:
			if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( m_currentVertexDataMask, MESH_VERTEX_ATTRIBUTE_UV1 ) != 0 )
			{
				//By reading per-member, supports Endian conversions.
				for ( Vertex3D_Superset& vertex : m_currentVertices )
				{
					didRead = reader.Read<float>( &vertex.m_texCoords1.x );
					didRead = reader.Read<float>( &vertex.m_texCoords1.y );
				}
			}
			break;
		case MESH_VERTEX_ATTRIBUTE_UV2:
			if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( m_currentVertexDataMask, MESH_VERTEX_ATTRIBUTE_UV2 ) != 0 )
			{
				//By reading per-member, supports Endian conversions.
				for ( Vertex3D_Superset& vertex : m_currentVertices )
				{
					didRead = reader.Read<float>( &vertex.m_texCoords2.x );
					didRead = reader.Read<float>( &vertex.m_texCoords2.y );
				}
			}
			break;
		case MESH_VERTEX_ATTRIBUTE_UV3:
			if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( m_currentVertexDataMask, MESH_VERTEX_ATTRIBUTE_UV3 ) != 0 )
			{
				//By reading per-member, supports Endian conversions.
				for ( Vertex3D_Superset& vertex : m_currentVertices )
				{
					didRead = reader.Read<float>( &vertex.m_texCoords3.x );
					didRead = reader.Read<float>( &vertex.m_texCoords3.y );
				}
			}
			break;
		case MESH_VERTEX_ATTRIBUTE_TANGENT:
			if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( m_currentVertexDataMask, MESH_VERTEX_ATTRIBUTE_TANGENT ) != 0 )
			{
				//By reading per-member, supports Endian conversions.
				for ( Vertex3D_Superset& vertex : m_currentVertices )
				{
					didRead = reader.Read<float>( &vertex.m_tangent.y );
					didRead = reader.Read<float>( &vertex.m_tangent.x );
					didRead = reader.Read<float>( &vertex.m_tangent.z );
				}
			}
			break;
		case MESH_VERTEX_ATTRIBUTE_BITANGENT:
			if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( m_currentVertexDataMask, MESH_VERTEX_ATTRIBUTE_BITANGENT ) != 0 )
			{
				//By writing per-member, supports Endian conversions.
				for ( Vertex3D_Superset& vertex : m_currentVertices )
				{
					didRead = reader.Read<float>( &vertex.m_bitangent.x );
					didRead = reader.Read<float>( &vertex.m_bitangent.y );
					didRead = reader.Read<float>( &vertex.m_bitangent.z );
				}
			}
			break;
		case MESH_VERTEX_ATTRIBUTE_NORMAL:
			if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( m_currentVertexDataMask, MESH_VERTEX_ATTRIBUTE_NORMAL ) != 0 )
			{
				//By reading per-member, supports Endian conversions.
				for ( Vertex3D_Superset& vertex : m_currentVertices )
				{
					didRead = reader.Read<float>( &vertex.m_normal.x );
					didRead = reader.Read<float>( &vertex.m_normal.y );
					didRead = reader.Read<float>( &vertex.m_normal.z );
				}
			}
			break;
		case MESH_VERTEX_ATTRIBUTE_SKINWEIGHTS:
			if ( GET_BIT_AT_BITFIELD_INDEX_MASKED( m_currentVertexDataMask, MESH_VERTEX_ATTRIBUTE_SKINWEIGHTS ) != 0 )
			{
				//By reading per-member, supports Endian conversions.
				for ( Vertex3D_Superset& vertex : m_currentVertices )
				{
					didRead = reader.Read<uint32_t>( &vertex.m_jointIndices.x );
					didRead = reader.Read<uint32_t>( &vertex.m_jointIndices.y );
					didRead = reader.Read<uint32_t>( &vertex.m_jointIndices.z );
					didRead = reader.Read<uint32_t>( &vertex.m_jointIndices.w );

					didRead = reader.Read<float>( &vertex.m_boneWeights.x );
					didRead = reader.Read<float>( &vertex.m_boneWeights.y );
					didRead = reader.Read<float>( &vertex.m_boneWeights.z );
					didRead = reader.Read<float>( &vertex.m_boneWeights.w );
				}
			}
			break;
		}

	}

	return didRead;
}


//--------------------------------------------------------------------------------------------------------------
bool MeshBuilder::ReadIndices( BinaryReader& reader )
{
	bool didRead = false;

	//The problem: ( reader.ReadBytes( m_currentIndices.data(), indicesCount ) == indicesCount );
		//Only reads # bytes == # indices, but sizeof(uint) != 1 byte.
		//Furthermore, for Endian mode conversion, need to read member by member, not struct by struct.

	for ( unsigned int& index : m_currentIndices )
		didRead = reader.Read<uint32_t>( &index );

	return didRead;
}





//--------------------------------------------------------------------------------------------------------------
bool MeshBuilder::ReadDrawInstructions( BinaryReader& reader )
{
	bool didRead = false;

	//The problem: ( reader.ReadBytes( m_currentInstructions.data(), instructionsCount ) == instructionsCount );
		//Only reads # bytes == # instructions, but sizeof an instruction != 1 byte.
		//Furthermore, for Endian mode conversion, need to read member by member, not struct by struct.

	for ( DrawInstruction& instruction : m_currentInstructions )
	{
		didRead = reader.Read<VertexGroupingRule>( &instruction.m_type );
		didRead = reader.Read<uint32_t>( &instruction.m_startIndex );
		didRead = reader.Read<uint32_t>( &instruction.m_count );
		didRead = reader.Read<uint32_t>( &instruction.m_usingIndexBuffer );
	}

	return didRead;
}


//--------------------------------------------------------------------------------------------------------------
uint32_t MeshBuilder::ReadVertexDataMask( BinaryReader& reader )
{
	//When we read it, we read in a string, then a flag corresponding to the string, and update/add to our read-mask.
	uint32_t mask = 0;
	std::string str = ""; //char* caused problems with going out of scope in ReadString.
	reader.ReadString( str );
	while ( str != "" )
	{
		//string::compare used to get rid of the \0 difference between them.
		if ( str.compare( 0, str.size()-1, VertexDefinition::VERTEX_ATTRIBUTE_NAME_POSITION ) == 0 )
			mask |= GET_BIT_AT_BITFIELD_INDEX( MESH_VERTEX_ATTRIBUTE_POSITION );
		else if ( str.compare( 0, str.size()-1, VertexDefinition::VERTEX_ATTRIBUTE_NAME_COLOR ) == 0 )
			mask |= GET_BIT_AT_BITFIELD_INDEX( MESH_VERTEX_ATTRIBUTE_COLOR );
		else if ( str.compare( 0, str.size()-1, VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV0 ) == 0 )
			mask |= GET_BIT_AT_BITFIELD_INDEX( MESH_VERTEX_ATTRIBUTE_UV0 );
		else if ( str.compare( 0, str.size()-1, VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV1 ) == 0 )
			mask |= GET_BIT_AT_BITFIELD_INDEX( MESH_VERTEX_ATTRIBUTE_UV1 );
		else if ( str.compare( 0, str.size()-1, VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV2 ) == 0 )
			mask |= GET_BIT_AT_BITFIELD_INDEX( MESH_VERTEX_ATTRIBUTE_UV2 );
		else if ( str.compare( 0, str.size()-1, VertexDefinition::VERTEX_ATTRIBUTE_NAME_UV3 ) == 0 )
			mask |= GET_BIT_AT_BITFIELD_INDEX( MESH_VERTEX_ATTRIBUTE_UV3 );
		else if ( str.compare( 0, str.size()-1, VertexDefinition::VERTEX_ATTRIBUTE_NAME_TANGENT ) == 0 )
			mask |= GET_BIT_AT_BITFIELD_INDEX( MESH_VERTEX_ATTRIBUTE_TANGENT );
		else if ( str.compare( 0, str.size()-1, VertexDefinition::VERTEX_ATTRIBUTE_NAME_BITANGENT ) == 0 )
			mask |= GET_BIT_AT_BITFIELD_INDEX( MESH_VERTEX_ATTRIBUTE_BITANGENT );
		else if ( str.compare( 0, str.size() - 1, VertexDefinition::VERTEX_ATTRIBUTE_NAME_NORMAL ) == 0 )
			mask |= GET_BIT_AT_BITFIELD_INDEX( MESH_VERTEX_ATTRIBUTE_NORMAL );
		else if ( str.compare( 0, str.size() - 1, VertexDefinition::VERTEX_ATTRIBUTE_NAME_SKINWEIGHTS ) == 0 )
			mask |= GET_BIT_AT_BITFIELD_INDEX( MESH_VERTEX_ATTRIBUTE_SKINWEIGHTS );
		else 
			ERROR_AND_DIE( "Unsupported string found in MeshBuilder::ReadVertexDataMask!" );

		reader.ReadString( str );
	}

	return mask;
}
