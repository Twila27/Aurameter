#pragma once

#include "Engine/EngineCommon.hpp"
#include "Engine/Renderer/Vertexes.hpp"


//-----------------------------------------------------------------------------
class MeshBuilder;
extern MeshBuilder* g_lastLoadedMeshBuilder; //Used for saving out built meshes.


//-----------------------------------------------------------------------------
struct Rgba;
struct DrawInstruction;
class Mesh;
class BinaryWriter;
class BinaryReader;
class Command;
class VertexDefinition;


//-----------------------------------------------------------------------------
enum MeshVertexAttributeBitIndex { //For data mask.
	MESH_VERTEX_ATTRIBUTE_POSITION = 0,
	MESH_VERTEX_ATTRIBUTE_COLOR,
	MESH_VERTEX_ATTRIBUTE_UV0,
	MESH_VERTEX_ATTRIBUTE_UV1,
	MESH_VERTEX_ATTRIBUTE_UV2,
	MESH_VERTEX_ATTRIBUTE_UV3,
	MESH_VERTEX_ATTRIBUTE_TANGENT,
	MESH_VERTEX_ATTRIBUTE_BITANGENT,
	MESH_VERTEX_ATTRIBUTE_NORMAL,
	MESH_VERTEX_ATTRIBUTE_SKINWEIGHTS,
	NUM_MESH_VERTEX_ATTRIBUTES
};


//-----------------------------------------------------------------------------
class MeshBuilder
{
public:

	//See Surface Patch generation or FBXUtils code for usage.
	void Begin( VertexGroupingRule primitiveType, uint32_t usingIndexBuffer );
	void SetColor( const Rgba& color ) { m_currentTemplate.m_color = color; }
	void SetTangent( const Vector3f& tangent ) { m_currentTemplate.m_tangent = tangent; }
	void SetBitangent( const Vector3f& bitangent ) { m_currentTemplate.m_bitangent = bitangent; }
	void SetNormal( const Vector3f& normal ) { m_currentTemplate.m_normal = normal; }
	void SetUV0( const Vector2f& uv ) { m_currentTemplate.m_texCoords0 = uv; }
	void SetUV1( const Vector2f& uv ) { m_currentTemplate.m_texCoords1 = uv; }
	void SetTBN( const Vector3f& tangent, const Vector3f& bitangent, const Vector3f& normal ) { SetTangent(tangent); SetBitangent(bitangent); SetNormal(normal); }
	void SetSkinWeights( const Vector4<unsigned int>& jointIndices, const Vector4f& boneWeights );
	void ClearBoneWeights();
	void End();
	std::string GetMaterialID() const { return m_currentMaterialID; }
	void SetMaterialID( const char* name ) { m_currentMaterialID = name; }
	void SetVertexDataMaskBit( MeshVertexAttributeBitIndex bitTag );
	void UnsetVertexDataMaskBit( MeshVertexAttributeBitIndex bitTag );
	uint32_t GetCurrentDataMask() const { return m_currentVertexDataMask; }

	//-----------------------------------------------------------------------------
	void AddVertex( const Vector3f& position ) { m_currentTemplate.m_position = position; m_currentVertices.push_back( m_currentTemplate ); }
	void AddIndicesForQuad( unsigned int topLeftIndex, unsigned int topRightIndex, unsigned int bottomLeftIndex, unsigned int bottomRightIndex );

	//-----------------------------------------------------------------------------
	void CopyToMesh( Mesh* mesh );

	//-----------------------------------------------------------------------------
	void BuildTriangle( const Vector3f& topLeft, const Vector3f& bottomLeft, const Vector3f& bottomRight ); //BL is (0,0), TR is (1,1).
	void BuildPlane( const Vector3f& initialPosition, const Vector3f& rightPlanarDirection, const Vector3f& upPlanarDirection, 
					 float startX, float endX, unsigned int xSectionCount, float startY, float endY, unsigned int ySectionCount );
	void BuildSurfacePatch( Vector3f( *surfaceEquation ) ( const Vector2f& position, const void* surfaceObject ), void* userData, 
							float startX, float endX, unsigned int xSectionCount, float startY, float endY, unsigned int ySectionCount );

	//-----------------------------------------------------------------------------
	unsigned int AppendMeshBuilderWhileMasksAndMaterialsMatch( const std::vector< MeshBuilder* >& others );
	bool AppendMeshBuilderIfMasksMatch( const MeshBuilder& other );

	//-----------------------------------------------------------------------------
	bool ReadFromFile( const char* filename, int endianMode );
	bool ReadFromStream( BinaryReader& reader );
	uint32_t ReadVertexDataMask( BinaryReader& reader );

	bool WriteToFile( const char* filename, bool appendToFile, int endianMode );
	bool WriteToStream( BinaryWriter& writer );
	bool WriteVertexDataMask( BinaryWriter& writer, uint32_t vertexDataMask );

	bool IsSkinned() const; //Skeletal versus static mesh.
	const VertexDefinition* GetVertexDefinitionFromVertexDataMask();
	void SetVertexDataMaskFromVertexDefinition( const VertexDefinition* vertexDefinition );


private:
	unsigned int m_currentStartIndex;
	VertexGroupingRule m_currentVertexGroupingRule;
	uint32_t m_usingIndexBuffer;
	std::string m_currentMaterialID;
	uint32_t m_currentVertexDataMask; //e.g. whether we have tangents in an instance's vertex data.

	Vertex3D_Superset m_currentTemplate; //the current "stamp" state.
	std::vector< Vertex3D_Superset > m_currentVertices;
	std::vector< unsigned int > m_currentIndices;
	std::vector< DrawInstruction > m_currentInstructions;

	static const uint32_t s_FILE_VERSION = 1;
	bool WriteVertices( BinaryWriter& writer );
	bool WriteIndices( BinaryWriter& writer );
	bool WriteDrawInstructions( BinaryWriter& writer );
	bool ReadVertices( BinaryReader& reader );
	bool ReadIndices( BinaryReader& reader );
	bool ReadDrawInstructions( BinaryReader& reader );
};


/* MeshBuilder Serialization Format v1.0 (AES A03: Pre-Bone Weights)
		1. FILE VERSION -- this way we don't try to load an old format, just forbid it.
		2. Material ID
		3. Vertex Data Mask (i.e. position, tangent, normal, etc... )
			When we do animation you'll want to have bone weights.
			i.e. We want to be able to support various vertex formats, leading us to use this mask.
			Each "bit" tells us whether we have position, tangent, normal data, etc.
				May not want to write a bitmask to the file, because changing the flags will corrupt all prior versions of the bitmask--unless you never reorder/remove it.
				e.g. Pos = bit 0, Normal = 1, UVs = 2, want to add Color between Pos and Normal would mess up older versions.
			Can store total num bits or # on bits--we will do this, storing tags rather than the direct bitmask.
				Engine will use bitmask, file format will use tags, to be future-proofed.
			File version only updated when they become incompatible--you want to write the format to not have this happen!
		4. Vertices
		5. Indices
		6. Draw Instructions
*/
