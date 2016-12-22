#include "Engine/Tools/FBXUtils.hpp"
#include "Engine/EngineCommon.hpp"
#include "Engine/Core/Command.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/Core/TheConsole.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/Skeleton.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Renderer/DebugRenderCommand.hpp"
#include "Engine/Math/MatrixStack.hpp"
#include "Engine/Renderer/AnimationSequence.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <string>

#if defined( TOOLS_BUILD )
#include <fbxsdk.h>
#include <algorithm>
#pragma comment( lib, "libfbxsdk-md.lib" ) //Where the code's coming from in the above #included file.

//The helper functions don't need stubbing because static--they're scoped to this .cpp exclusively.


//--------------------------------------------------------------------------------------------------------------
static const char* GetAttributeTypeName( FbxNodeAttribute::EType type )
{
	//These are all the types FBX supports. In AES, we will ignore all but skeleton or mesh. But these are all things an FBX scene can support.
	switch ( type )
	{
	case FbxNodeAttribute::eUnknown: return "unidentified";
	case FbxNodeAttribute::eNull: return "null";
	case FbxNodeAttribute::eMarker: return "marker";
	case FbxNodeAttribute::eSkeleton: return "skeleton";
	case FbxNodeAttribute::eMesh: return "mesh";
	case FbxNodeAttribute::eNurbs: return "nurbs";
	case FbxNodeAttribute::ePatch: return "patch";
	case FbxNodeAttribute::eCamera: return "camera";
	case FbxNodeAttribute::eCameraStereo: return "stereo";
	case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
	case FbxNodeAttribute::eLight: return "light";
	case FbxNodeAttribute::eOpticalReference: return "optical reference";
	case FbxNodeAttribute::eOpticalMarker: return "marker";
	case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
	case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
	case FbxNodeAttribute::eBoundary: return "boundary";
	case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
	case FbxNodeAttribute::eShape: return "shape";
	case FbxNodeAttribute::eLODGroup: return "lodgroup";
	case FbxNodeAttribute::eSubDiv: return "subdiv";
	default: return "unknown";
	}
	//For example, we could initialize a light or camera based on those cases.
}


//--------------------------------------------------------------------------------------------------------------
static void PrintAttribute( FbxNodeAttribute* attribute, int depth )
{
	static int indentScale = 4;

	if ( nullptr == attribute )
		return;

	FbxNodeAttribute::EType type = attribute->GetAttributeType();

	const char* typeName = GetAttributeTypeName( type );
	const char* attributeName = attribute->GetName();

	g_theConsole->Printf( "%*s- type='%s', name='%s'", depth * indentScale, " ", typeName, attributeName );
}


//--------------------------------------------------------------------------------------------------------------
static void PrintNode( FbxNode* node, int depth )
{
	static int indentScale = 4;

	g_theConsole->Printf( "%*sNode [%s]", depth * indentScale, " ", node->GetName() ); //Indents per node level.
																					   // %*s will print " " as many times as specified by depth.

	for ( int i = 0; i < node->GetNodeAttributeCount(); i++ )
	{
		PrintAttribute( node->GetNodeAttributeByIndex( i ), depth );
	}

	for ( int i = 0; i < node->GetChildCount(); i++ )
	{
		PrintNode( node->GetChild( i ), depth + 1 );
	}
}


//--------------------------------------------------------------------------------------------------------------
void FBXListSceneFromFile( const char* filename )
{
	UNREFERENCED( filename );

	FbxManager* fbxManager = FbxManager::Create();
	if ( nullptr == fbxManager )
	{
		DebuggerPrintf( "Could not create FBX manager." );
		return;
	}

	FbxIOSettings* io_settings = FbxIOSettings::Create( fbxManager, IOSROOT ); //Can provide names for things, but Forseth never uses it.
																			   //By default will load everything, so it's all we need to do.
																			   //To speed it up, can only load parts, but since this is an offline tool not a concern.
	fbxManager->SetIOSettings( io_settings );

	FbxImporter* importer = FbxImporter::Create( fbxManager, "" ); //Defines how the file we're reading is imported as a scene.

	std::string finalFilename = "Data/Models/";
	finalFilename += filename;
	bool loadSuccess = importer->Initialize( finalFilename.c_str(),
											 -1, // the file format - -1 lets it figure out for itself
											 fbxManager->GetIOSettings() // same as above pointer
											 );

	if ( loadSuccess )
	{
		FbxScene* scene = FbxScene::Create( fbxManager, "" );
		//Last argument is to name scene, makes sense upon generation, not on importing.

		bool importSuccess = importer->Import( scene ); //Converts above imported scene into the scene file.
		if ( importSuccess )
		{
			//Scene has no circular references, just a DAG tree--so we can print recursively.
			FbxNode* root = scene->GetRootNode();
			PrintNode( root, 0 ); //First print current's attributes, then depth starting at 0, then all children.
		}

		FBX_SAFE_DESTROY( scene );
	}
	else
	{
		g_theConsole->Printf( Stringf( "FBX SDK failed to import scene %s.", filename ).c_str() );
		g_theConsole->Printf( "Note code tries to load from Data/Models/ in active working directory, won't append .fbx." );
	}

	FBX_SAFE_DESTROY( importer );
	FBX_SAFE_DESTROY( io_settings );
	FBX_SAFE_DESTROY( fbxManager );
}


//--------------------------------------------------------------------------------------------------------------
static Vector3f ToEngineVec3( const FbxVector4& fbxVec4 )
{
	Vector3f engineVector;
	return Vector3f( (float)fbxVec4.mData[ 0 ],
					(float)fbxVec4.mData[ 1 ],
					(float)fbxVec4.mData[ 2 ] );
}


//--------------------------------------------------------------------------------------------------------------
static Vector4f ToEngineVec4( const FbxDouble4& fbxVec4 )
{
	Vector4f engineVector;
	return Vector4f( (float)fbxVec4.mData[ 0 ],
					(float)fbxVec4.mData[ 1 ],
					(float)fbxVec4.mData[ 2 ],
					(float)fbxVec4.mData[ 3 ] );
}


//--------------------------------------------------------------------------------------------------------------
static bool GetPositionForMeshAndVertex( Vector3f* out_position, const Matrix4x4f& transform, FbxMesh* mesh, int polyIndex, int vertexIndex )
{
	int controlPointIndex = mesh->GetPolygonVertex( polyIndex, vertexIndex );

	FbxVector4 fbxPosition = mesh->GetControlPointAt( controlPointIndex ); //a vec4 of doubles.
	
	Vector4f preTransformPosition = Vector4f( ToEngineVec3( fbxPosition ), 1.0f );
	*out_position = transform.TransformVector( preTransformPosition ).xyz();
	//Multiply [M] on right if the matrix is column-major, on left if the matrix is row-major.

	return true;
}


//--------------------------------------------------------------------------------------------------------------
template < typename ElementListType, typename AttributeType >
static bool GetMeshAttributeFromElementHelper( ElementListType* elementList, AttributeType* out_attribute, int elementIndex )
{
	switch ( elementList->GetReferenceMode() )
	{
	case FbxGeometryElement::eDirect: //Each control point has a non-duplicate attribute value.
		if ( elementIndex < elementList->GetDirectArray().GetCount() )
		{
			*out_attribute = elementList->GetDirectArray().GetAt( elementIndex ); //So we can access 1-to-1.
			return true;
		}
		break;
	case FbxGeometryElement::eIndexToDirect: //Each control point can access its attribute values via redirection index array.
		if ( elementIndex < elementList->GetIndexArray().GetCount() )
		{
			int redirectedIndex = elementList->GetIndexArray().GetAt( elementIndex ); //First grab result of redirection.
			*out_attribute = elementList->GetDirectArray().GetAt( redirectedIndex ); //Then index in based on that.
			return true;
		}
		break;
	}
	return false;
}


//--------------------------------------------------------------------------------------------------------------
template < typename ElementListType, typename AttributeType >
static bool GetMeshAttributeFromElement( FbxMesh* mesh, int polyIndex, int vertexIndex, ElementListType* elementList, AttributeType* out_attribute )
{
	if ( nullptr == elementList )
		return false;

	int elementIndex;
	switch ( elementList->GetMappingMode() )
	{
	case FbxGeometryElement::eByControlPoint:
		//Control point index we get is also indexing into this element to get the element. Unique per control point.
		elementIndex /*controlPointIndex*/ = mesh->GetPolygonVertex( polyIndex, vertexIndex );
		GetMeshAttributeFromElementHelper( elementList, out_attribute, elementIndex );
		break;
	case FbxGeometryElement::eByPolygonVertex:
		elementIndex /*directVertexIndex*/ = ( polyIndex * 3 ) + vertexIndex; //Only valid for meshes that have been triangulated like we do for them.
		GetMeshAttributeFromElementHelper( elementList, out_attribute, elementIndex );
		break;
	default: break;
	}

	return true;
}


//--------------------------------------------------------------------------------------------------------------
static bool GetBitangentForMeshAndVertex( Vector3f* out_bitangent, const Matrix4x4f& transform, FbxMesh* mesh, int polyIndex, int vertexIndex )
{
	UNREFERENCED( transform );

	FbxVector4 bitangent;
	FbxGeometryElementBinormal* bitangents = mesh->GetElementBinormal();
	//FBX stores things in a bunch of lists (called geometry elements).
	//These lists get indexed differently per list.

	if ( GetMeshAttributeFromElement<FbxGeometryElementBinormal, FbxVector4>( mesh, polyIndex, vertexIndex, bitangents, &bitangent ) )
	{
		Vector3f b = ToEngineVec3( bitangent );
		*out_bitangent = transform.TransformVector( Vector4f( b, 0.f ) ).xyz();
		out_bitangent->Normalize();
		return true;
	}

	return false; //Didn't exist.
}


//--------------------------------------------------------------------------------------------------------------
static bool GetTangentForMeshAndVertex( Vector3f* out_tangent, const Matrix4x4f& transform, FbxMesh* mesh, int polyIndex, int vertexIndex )
{
	UNREFERENCED( transform );

	FbxVector4 tangent;
	FbxGeometryElementTangent* tangents = mesh->GetElementTangent();
	//FBX stores things in a bunch of lists (called geometry elements).
	//These lists get indexed differently per list.

	if ( GetMeshAttributeFromElement<FbxGeometryElementTangent, FbxVector4>( mesh, polyIndex, vertexIndex, tangents, &tangent ) )
	{
		Vector3f t = ToEngineVec3( tangent );
		*out_tangent = transform.TransformVector( Vector4f( t, 0.f ) ).xyz();
		out_tangent->Normalize();
		return true;
	}

	return false; //Didn't exist.
}


//--------------------------------------------------------------------------------------------------------------
static bool GetNormalForMeshAndVertex( Vector3f* out_normal, const Matrix4x4f& transform, FbxMesh* mesh, int polyIndex, int vertexIndex )
{
	FbxVector4 normal;
	FbxGeometryElementNormal* normals = mesh->GetElementNormal();
	//FBX stores things in a bunch of lists (called geometry elements).
	//These lists get indexed differently per list.

	if ( GetMeshAttributeFromElement<FbxGeometryElementNormal, FbxVector4>( mesh, polyIndex, vertexIndex, normals, &normal ) )
	{
		Vector3f n = ToEngineVec3( normal );
		*out_normal = transform.TransformVector( Vector4f( n, 0.f ) ).xyz();
		out_normal->Normalize();
		return true;
	}

	return false; //Didn't exist.
}


//--------------------------------------------------------------------------------------------------------------
static bool GetColorForMeshAndVertex( Rgba* out_color, const Matrix4x4f& transform, FbxMesh* mesh, int polyIndex, int vertexIndex )
{
	UNREFERENCED( transform );

	FbxColor color;
	FbxGeometryElementVertexColor* colors = mesh->GetElementVertexColor();
	//FBX stores things in a bunch of lists (called geometry elements).
	//These lists get indexed differently per list.

	if ( GetMeshAttributeFromElement<FbxGeometryElementVertexColor, FbxColor>( mesh, polyIndex, vertexIndex, colors, &color ) )
	{
		*out_color = Rgba( (float)color.mRed, (float)color.mGreen, (float)color.mBlue, (float)color.mAlpha );
		return true;
	}

	return false; //Didn't exist.
}


//--------------------------------------------------------------------------------------------------------------
static bool GetUVForMeshAndVertex( Vector2f* out_uv, FbxMesh* mesh, int polyIndex, int vertexIndex, int uvIndex )
{
	FbxVector2 uv;
	FbxGeometryElementUV* uvs = mesh->GetElementUV( uvIndex );
	//FBX stores things in a bunch of lists (called geometry elements).
	//These lists get indexed differently per list.

	if ( GetMeshAttributeFromElement<FbxGeometryElementUV, FbxVector2>( mesh, polyIndex, vertexIndex, uvs, &uv ) )
	{
		*out_uv = Vector2f( (float)uv.mData[ 0 ], (float)uv.mData[ 1 ] );
		return true;
	}

	return false; //Didn't exist.
}


//--------------------------------------------------------------------------------------------------------------
static bool HasSkinWeights( const FbxMesh* mesh )
{
	TODO( " Morph Targets! " );
	//To support morph target animation, you'll need to also import all submeshes and get the curve info off the deformer for how it's evaluating/blending those submeshes together.
	return mesh->GetDeformerCount( FbxDeformer::eSkin ) > 0;
}


//--------------------------------------------------------------------------------------------------------------
static void KeepHighestWeights( SkinWeight* skinWeight, int jointIndex, float weight )
{
	float &x = skinWeight->boneWeights.x;
	float &y = skinWeight->boneWeights.y;
	float &z = skinWeight->boneWeights.z;
	float &w = skinWeight->boneWeights.w;
	//Go through skinWeight, if float weight is higher than the lowest value in skinWeight, replace it, else return changing nothing.
	if ( w < z )
	{
		if ( w < y )
		{
			if ( w < x )
			{
				if ( w < weight )
				{
					skinWeight->jointIndices.w = jointIndex;
					w = weight;
				}
				return;
			}
			else
			{
				if ( x < weight )
				{
					skinWeight->jointIndices.x = jointIndex;
					x = weight;
				}
				return;
			}
		}
		else
		{
			if ( y < x )
			{
				if ( y < weight )
				{
					skinWeight->jointIndices.y = jointIndex;
					y = weight;
				}
				return;
			}
			else
			{
				if ( x < weight )
				{
					skinWeight->jointIndices.x = jointIndex;
					x = weight;
				}
				return;
			}
		}
	}
	else
	{
		if ( z < y )
		{
			if ( z < x )
			{
				if ( z < weight )
				{
					skinWeight->jointIndices.z = jointIndex;
					z = weight;
				}
				return;
			}
			else
			{
				if ( x < weight )
				{
					skinWeight->jointIndices.x = jointIndex;
					x = weight;
				}
				return;
			}
		}
		else
		{
			if ( y < x )
			{
				if ( y < weight )
				{
					skinWeight->jointIndices.y = jointIndex;
					y = weight;
				}
				return;
			}
			else
			{
				if ( x < weight )
				{
					skinWeight->jointIndices.x = jointIndex;
					x = weight;
				}
				return;
			}
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
static int GetJointIndexForFbxNode( const std::map< int, FbxNode* > &jointToNodeMapping, FbxNode* linkNode )
{
	int jointIndex = INVALID_JOINT_INDEX;
	for ( auto iter = jointToNodeMapping.begin(); iter != jointToNodeMapping.end(); ++iter )
	{
		if ( iter->second == linkNode )
		{
			jointIndex = iter->first;
			break;
		}
	}

	return jointIndex;
}

//--------------------------------------------------------------------------------------------------------------
static void GetSkinWeightsForMesh( std::vector<SkinWeight>& out_skinWeights, const FbxMesh* mesh, const std::map< int, FbxNode* >& jointToNodeMapping )
{
	for ( size_t i = 0; i < out_skinWeights.size(); ++i ) {
		out_skinWeights[ i ].jointIndices = Vector4<unsigned int>( 0 );
		out_skinWeights[ i ].boneWeights = Vector4f( 0.f );
	}

	int deformerCount = mesh->GetDeformerCount( FbxDeformer::eSkin );
		//Should almost always be 1, as there's typically only 1 skin per mesh.
	for ( int deformerIndex = 0; deformerIndex < deformerCount; deformerIndex++ )
	{
		FbxSkin* skin = (FbxSkin*)mesh->GetDeformer( deformerIndex, FbxDeformer::eSkin );
		if ( skin == nullptr )
			continue;

		//Each skin element is made up of clusters for how things associate with a bone.
		//For each bone included with this mesh, you have another cluster, and you get the weights in each.
		//Cluster count should be <= # bones.
		int clusterCount = skin->GetClusterCount();
		for ( int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++ )
		{
			FbxCluster* cluster = skin->GetCluster( clusterIndex );

			FbxNode* linkNode = cluster->GetLink(); //What bone this cluster's associated to.
			if ( linkNode == nullptr )
				continue; //Nothing associated, do nothing.

			int jointIndex = GetJointIndexForFbxNode( jointToNodeMapping, linkNode );
			if ( jointIndex == INVALID_JOINT_INDEX )
				continue;

			int* controlPointIndices = cluster->GetControlPointIndices();
			int indexCount = cluster->GetControlPointIndicesCount();
			double* weights = cluster->GetControlPointWeights();

			for ( int i = 0; i < indexCount; i++ )
			{
				int controlIndex = controlPointIndices[ i ];
				double weight = weights[ i ];

				SkinWeight* skinWeight = &out_skinWeights[ controlIndex ];
				KeepHighestWeights( skinWeight, jointIndex, (float)weight );
			}
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
static void ImportVertex( MeshBuilder* mb, const Matrix4x4f& transform, FbxMesh* mesh, int polyIndex, int vertexIndex, std::vector<SkinWeight>& out_skinWeights )
{
	//Each actual vertex is uniquely defined by a vertex 0, 1, or 2 and a polygon index.

	Vector3f tangent;
	if ( GetTangentForMeshAndVertex( &tangent, transform, mesh, polyIndex, vertexIndex ) )
		mb->SetTangent( tangent );

	Vector3f bitangent;
	if ( GetBitangentForMeshAndVertex( &bitangent, transform, mesh, polyIndex, vertexIndex ) )
		mb->SetBitangent( bitangent );

	Vector3f normal;
	if ( GetNormalForMeshAndVertex( &normal, transform, mesh, polyIndex, vertexIndex ) )
		mb->SetNormal( normal );

	Vector2f uv0;
	if ( GetUVForMeshAndVertex( &uv0, mesh, polyIndex, vertexIndex, 0 ) )
		//0 to only get uv0. Can have more UV pairs. Can loop more.
		//0-2 is a good practice to support that many.
		//0-4 is all you'd ever really need.
		mb->SetUV0( Vector2f( uv0.x, 1 - uv0.y ) ); //Note inverted-y, for OpenGL!

	Vector2f uv1;
	if ( GetUVForMeshAndVertex( &uv1, mesh, polyIndex, vertexIndex, 1 ) )
		mb->SetUV1( uv1 );

	Rgba color;
	if ( GetColorForMeshAndVertex( &color, transform, mesh, polyIndex, vertexIndex ) )
		mb->SetColor( color );

	unsigned int controlIndex = mesh->GetPolygonVertex( polyIndex, vertexIndex );
	if ( controlIndex < out_skinWeights.size() )
	{
		SkinWeight& skinWeight = out_skinWeights[ controlIndex ];

		//A vertex can't have more or less than a total of 100% weighted-ness.
		float totalWeight = skinWeight.boneWeights.x + skinWeight.boneWeights.y + skinWeight.boneWeights.z + skinWeight.boneWeights.w;
		if ( totalWeight != 0 )
		{
			skinWeight.boneWeights.x /= totalWeight;
			skinWeight.boneWeights.y /= totalWeight;
			skinWeight.boneWeights.z /= totalWeight;
			skinWeight.boneWeights.w /= totalWeight;
		}
		else
			skinWeight.boneWeights = Vertex3D_Superset::DEFAULT_BONE_WEIGHTS;

		mb->SetSkinWeights( //Note renormalizing inside to be safe.
			out_skinWeights[ controlIndex ].jointIndices,
			out_skinWeights[ controlIndex ].boneWeights );
	}
	else
		mb->ClearBoneWeights();

	Vector3f position;
	if ( GetPositionForMeshAndVertex( &position, transform, mesh, polyIndex, vertexIndex ) )
		mb->AddVertex( position ); //Must come last after above setters or their settings won't take when adding new vertex.
}


//--------------------------------------------------------------------------------------------------------------
//FBX stores in row-major, memory along rows. Have to do more if your default matrix ordering is along columns in memory.
static Matrix4x4f ToEngineMatrixFromFbxRowMajorMatrix( const FbxMatrix& fbxMatrix, Ordering resultOrdering )
{
	Matrix4x4f engineMatrix;
	engineMatrix.SetRows(
		ToEngineVec4( fbxMatrix.mData[ 0 ] ),
		ToEngineVec4( fbxMatrix.mData[ 1 ] ),
		ToEngineVec4( fbxMatrix.mData[ 2 ] ),
		ToEngineVec4( fbxMatrix.mData[ 3 ] )
		);

	if ( resultOrdering != engineMatrix.GetOrdering() )
		engineMatrix.SetToTranspose( true );

	return engineMatrix;
}


//--------------------------------------------------------------------------------------------------------------
static Matrix4x4f GetGeometricTransform( FbxNode* node )
{
	if ( ( node != nullptr ) && ( node->GetNodeAttribute() != nullptr ) )
	{
		const FbxVector4 geometricTranslation = node->GetGeometricTranslation( FbxNode::eSourcePivot );
		const FbxVector4 geometricRotation = node->GetGeometricRotation( FbxNode::eSourcePivot );
		const FbxVector4 geometricScale = node->GetGeometricScaling( FbxNode::eSourcePivot );

		FbxMatrix geometricTransform;
		geometricTransform.SetTRS( geometricTranslation, geometricRotation, geometricScale );

		return ToEngineMatrixFromFbxRowMajorMatrix( geometricTransform, ROW_MAJOR );
	}
	else return Matrix4x4f::IDENTITY;
}


//--------------------------------------------------------------------------------------------------------------
static void ImportMesh( ImportedScene* importedScene, //What we're importing into
						FbxMesh* mesh, //Object we're importing
						Matrix4x4Stack& matrixStack, //Tracks current Transform
						const std::map< int, FbxNode* >& jointToNodeMapping )
{
	//Special kind of 3DS Max-only transforms applying just to this node, not its children.
	Matrix4x4f geometricTransform = GetGeometricTransform( mesh->GetNode() );
	matrixStack.Push( geometricTransform );
	Matrix4x4f transform = matrixStack.Peek();

	size_t controlPointCount = mesh->GetControlPointsCount();
	std::vector< SkinWeight > out_skinWeights;
	out_skinWeights.resize( controlPointCount );
	if ( HasSkinWeights( mesh ) )
	{
		GetSkinWeightsForMesh( out_skinWeights, mesh, jointToNodeMapping );
	}
	else //Handle unskinned meshes by parenting them under a joint that is skinned.
	{
		FbxNode* node = mesh->GetNode();
		FbxNode* parent = node->GetParent();
		//DO ACTUAL WALKING HERE -- walk up until we find an FbxNode that was in our mapping, 
		//since skeletons were already imported before this mapping will be filled out.

		while ( parent != nullptr )
		{
			int jointIndex = GetJointIndexForFbxNode( jointToNodeMapping, parent );

			if ( jointIndex != INVALID_JOINT_INDEX )
			{
				for ( SkinWeight& sw : out_skinWeights ) //AKA for all vertices.
				{
					sw.jointIndices = Vector4<unsigned int>( jointIndex, 0, 0, 0 );
					sw.boneWeights = Vector4f( 1.f, 0.f, 0.f, 0.f ); //Weight fully to jointIndex.
				}
				break;
			}

			node = parent;
			parent = node->GetParent();
		}
	}
	// There's no exception for static meshes right now, 
	//so every mesh is being treated like skeletal meshes, 
	//parenting everything to its parents...

	MeshBuilder* mb = new MeshBuilder();

	FbxSurfaceMaterial* meshMaterial = mesh->GetNode()->GetMaterial( 0 );
	if ( nullptr != meshMaterial )
		mb->SetMaterialID( meshMaterial->GetName() );

	ASSERT_OR_DIE( mesh->IsTriangleMesh(), "Mesh Isn't Triangle Mesh!" ); //IsTriangleMesh is native.		

	mb->Begin( VertexGroupingRule::AS_TRIANGLES, false ); //False = no IBO, as FBX has no concept of indices. 
		//You'll get a lot of dupes, you have to search and replace for them with indices. Once imported, would keep it with index buffers.
	
	//Actual mesh import:
	int polyCount = mesh->GetPolygonCount(); //We know they're all tris now, so polyCount == # tris.
	for ( int polyIndex = 0; polyIndex < polyCount; polyIndex++ )
	{
		int verticesTotal = mesh->GetPolygonSize( polyIndex );
		ASSERT_OR_DIE( verticesTotal == 3, "Found Unexpected Non-Triangle in ImportMesh!" ); //Sanity check, should be triangle mesh.

		for ( int vertexIndex = 0; vertexIndex < verticesTotal; vertexIndex++ )
			ImportVertex( mb, transform, mesh, polyIndex, vertexIndex, out_skinWeights );
	}

	mb->End();

	//Setup data mask.

	mb->SetVertexDataMaskBit( MESH_VERTEX_ATTRIBUTE_POSITION ); //Assume positions exist.
	
	if ( nullptr != mesh->GetElementVertexColor() )
		mb->SetVertexDataMaskBit( MESH_VERTEX_ATTRIBUTE_COLOR );

	if ( nullptr != mesh->GetElementUV( 0 ) )
		mb->SetVertexDataMaskBit( MESH_VERTEX_ATTRIBUTE_UV0 );
	if ( nullptr != mesh->GetElementUV( 1 ) )
		mb->SetVertexDataMaskBit( MESH_VERTEX_ATTRIBUTE_UV1 );
	if ( nullptr != mesh->GetElementUV( 2 ) )
		mb->SetVertexDataMaskBit( MESH_VERTEX_ATTRIBUTE_UV2 );
	if ( nullptr != mesh->GetElementUV( 3 ) )
		mb->SetVertexDataMaskBit( MESH_VERTEX_ATTRIBUTE_UV3 );

	if ( nullptr != mesh->GetElementTangent() )
		mb->SetVertexDataMaskBit( MESH_VERTEX_ATTRIBUTE_TANGENT );
	if ( nullptr != mesh->GetElementBinormal() )
		mb->SetVertexDataMaskBit( MESH_VERTEX_ATTRIBUTE_BITANGENT );
	if ( nullptr != mesh->GetElementNormal() )
		mb->SetVertexDataMaskBit( MESH_VERTEX_ATTRIBUTE_NORMAL );
	
	if ( HasSkinWeights( mesh ) )
		mb->SetVertexDataMaskBit( MESH_VERTEX_ATTRIBUTE_SKINWEIGHTS ); //Write out both fields when true!

	matrixStack.Pop();

	importedScene->m_meshBuilders.push_back( mb );
}


//--------------------------------------------------------------------------------------------------------------
static void TriangulateScene( FbxScene* scene )
{
	FbxGeometryConverter converter( scene->GetFbxManager() );
	converter.Triangulate( scene, true ); //Boolean affirms overwriting meshes in-place with their triangulated result.
}



//--------------------------------------------------------------------------------------------------------------
static Matrix4x4f GetNodeLocalTransform( FbxNode* node )
{
	//FbxMatrix fbxGlobalMatrix = node->EvaluateGlobalTransform();
		//We're using a matrix stack, so we need to use local transforms.
	FbxMatrix fbxLocalMatrix = node->EvaluateLocalTransform(); 
		//Doesn't work well with the matrix stack which inherently imposes global transform results.

	return ToEngineMatrixFromFbxRowMajorMatrix( fbxLocalMatrix, ROW_MAJOR );
}


//--------------------------------------------------------------------------------------------------------------
static void ImportSceneMeshes( ImportedScene* importedScene, // What's imported into
							 FbxNode* currentNode, //Object being imported
							 Matrix4x4Stack& matrixStack, //Tracks current Transform
							 const std::map< int, FbxNode* >& jointToNodeMapping )
{
	if ( currentNode == nullptr )
		return;

	Matrix4x4f nodeLocalTransform = GetNodeLocalTransform( currentNode );
	matrixStack.Push( nodeLocalTransform );

	//Walk attributes of the current node, looking for meshes.
	int numAttributes = currentNode->GetNodeAttributeCount();
	for ( int attributeIndex = 0; attributeIndex < numAttributes; attributeIndex++ )
	{
		FbxNodeAttribute* attribute = currentNode->GetNodeAttributeByIndex( attributeIndex );
		if ( attribute == nullptr )
			continue;

		if ( attribute->GetAttributeType() == FbxNodeAttribute::eMesh )
			ImportMesh( importedScene, (FbxMesh*)attribute, matrixStack, jointToNodeMapping );
	}

	//Do the rest of the tree recursively.
	int numChildren = currentNode->GetChildCount();
	for ( int childIndex = 0; childIndex < numChildren; childIndex++ )
		ImportSceneMeshes( importedScene, currentNode->GetChild( childIndex ), matrixStack, jointToNodeMapping );

	//Remove transform from stack.
	matrixStack.Pop();
}


//--------------------------------------------------------------------------------------------------------------
static Skeleton* ImportSkeleton( ImportedScene* importedScene, Matrix4x4Stack& matrixStackOfLocalTransforms, Skeleton* currentSkeleton, int parentJointIndex, FbxSkeleton* fbxSkeleton, std::map< int, FbxNode* >& jointToNodeMapping )
{
	Skeleton* returnedSkeleton = nullptr;
	if ( fbxSkeleton->IsSkeletonRoot() ) //True for new skeletons.
	{
		returnedSkeleton = new Skeleton();
		importedScene->m_skeletons.push_back( returnedSkeleton );
	}
	else
	{
		returnedSkeleton = currentSkeleton;
		ASSERT_OR_DIE( returnedSkeleton != nullptr, nullptr );
		//May trigger if FBXSDK acts up, implies trying to add a child with no parent to append it under.
	}

	Matrix4x4f geoTransform = GetGeometricTransform( fbxSkeleton->GetNode() );
	matrixStackOfLocalTransforms.Push( geoTransform );

	Matrix4x4f currentBoneToModelSpaceLocalTransform = matrixStackOfLocalTransforms.Peek();
	returnedSkeleton->AddJoint( fbxSkeleton->GetNode()->GetName(), parentJointIndex, currentBoneToModelSpaceLocalTransform );
	jointToNodeMapping.insert( std::pair< int, FbxNode* >( returnedSkeleton->GetLastAddedJointIndex(), fbxSkeleton->GetNode() ) );

	matrixStackOfLocalTransforms.Pop();
	return returnedSkeleton;
}


//--------------------------------------------------------------------------------------------------------------
static void ImportSceneSkeletons( ImportedScene* importedScene, FbxNode* currentNode, Matrix4x4Stack& matrixStackWithCurrentGlobalTransformOnTop, Skeleton* currentSkeleton, int parentJointIndex, std::map< int, FbxNode* >& jointToNodeMapping )
{
	if ( currentNode == nullptr )
		return;

	Matrix4x4f mat = GetNodeLocalTransform( currentNode );
	matrixStackWithCurrentGlobalTransformOnTop.Push( mat );

	//Walk attributes of the current node, looking for skeletons.
	int numAttributes = currentNode->GetNodeAttributeCount();
	for ( int attributeIndex = 0; attributeIndex < numAttributes; attributeIndex++ )
	{
		FbxNodeAttribute* attribute = currentNode->GetNodeAttributeByIndex( attributeIndex );
		if ( attribute == nullptr )
			continue;

		if ( attribute->GetAttributeType() == FbxNodeAttribute::eSkeleton )
		{
			Skeleton* newEngineSkeleton = ImportSkeleton( importedScene,
													   matrixStackWithCurrentGlobalTransformOnTop,
													   currentSkeleton,
													   parentJointIndex, //Calling it joint instead of bone index, because it's the actual joint position.
													   (FbxSkeleton*)attribute, //Adding itself as a child.
														jointToNodeMapping ); 
			
			// newSkeleton will either be the same skeleton passed in,
			// or a new skeleton, or no skeleton if it was a bad node.
			// If we got something back, we add it to our skeleton class instance's developing hierarchy.
			if ( newEngineSkeleton != nullptr )
			{
				currentSkeleton = newEngineSkeleton;
				parentJointIndex = currentSkeleton->GetLastAddedJointIndex();
			}
		}
	}

	//Do the rest of the tree recursively.
	int childCount = currentNode->GetChildCount();
	for ( int childIndex = 0; childIndex < childCount; childIndex++ )
		ImportSceneSkeletons( importedScene, currentNode->GetChild( childIndex ), matrixStackWithCurrentGlobalTransformOnTop, currentSkeleton, parentJointIndex, jointToNodeMapping );

	matrixStackWithCurrentGlobalTransformOnTop.Pop();
}


//--------------------------------------------------------------------------------------------------------------
static Matrix4x4f GetFbxNodeGlobalModelSpaceTransformAtTime( FbxNode* node, FbxTime time, const Matrix4x4f& importBaseTransform )
{
	if ( node == nullptr )
		return Matrix4x4f( ROW_MAJOR );

	FbxMatrix fbxMatrix = node->EvaluateGlobalTransform( time );
	Matrix4x4f engineMatrix = ToEngineMatrixFromFbxRowMajorMatrix( fbxMatrix, ROW_MAJOR );
	return engineMatrix * importBaseTransform; //!\ Be wary of ordering!
}


//--------------------------------------------------------------------------------------------------------------
static Matrix4x4f GetFbxNodeLocalJointSpaceTransformAtTime( FbxNode* node, FbxTime time, const Matrix4x4f& importBaseTransform )
{
	if ( node == nullptr )
		return Matrix4x4f( ROW_MAJOR );

	FbxMatrix fbxMatrix = node->EvaluateLocalTransform( time );
	Matrix4x4f engineMatrix = ToEngineMatrixFromFbxRowMajorMatrix( fbxMatrix, ROW_MAJOR );
	return engineMatrix * importBaseTransform; //!\ Be wary of ordering!
}


//--------------------------------------------------------------------------------------------------------------
static void ImportMotions( ImportedScene* importedScene, Matrix4x4f& importBaseTransform, std::map< int, FbxNode* >& jointToNodeMapping, FbxScene* scene, float framerate, bool useLocalOverGlobal )
{
	int numAnimationsInScene = scene->GetSrcObjectCount<FbxAnimStack>(); //# animations == # anim stacks, because 1 stack per 1 animation.

	if ( numAnimationsInScene == 0 || importedScene->m_skeletons.size() == 0 )
		return; //Can't import motions without skeletons.

	//Timing info for animation in this scene--scene basis, start/end times for motions, the timescale everything uses to scale rotation order, all global across-file stuff.
//Unused:	FbxGlobalSettings& settings = scene->GetGlobalSettings();
//Unused:	FbxTime::EMode timeMode = settings.GetTimeMode(); //Keyframes per second defined by this time mode, BUT could be a custom mode.
		//e.g. 24FPS for TV, 30 for game animation, 60 for high fidelity, etc. 

//Unused:	double sceneFramerate = ( timeMode == FbxTime::eCustom ) ? settings.GetCustomFrameRate() : FbxTime::GetFrameRate( timeMode );

	uint32_t numSkeletonsInScene = importedScene->m_skeletons.size();
	ASSERT_OR_DIE( numSkeletonsInScene == 1, nullptr ); //"An aggressive todo".
		//For now, this is all we'll need.
		//May have more than it in cinematics or Dark Souls visceral 2-person or 2-person wrestling moves. 
		//Very game-specific whether you need to support these.
		//If you do it, just figure when you're importing the FbxNode which FbxSkeleton it corresponds to.
			//e.g. If a skeleton has 8 bones and I'm on the 9th, it's bone #1 of skeleton 2.

	Skeleton* skeleton = importedScene->m_skeletons.front();
	//Walking motions at a fixed rate, unless you're doing animation curves.
	FbxTime advance; //Time between frames.
	advance.SetSecondDouble( static_cast<double>( 1.f / framerate ) ); //This is NOT sceneFramerate, but OUR framerate we pass into this function.

	//Note that this is done for one FBX scene file--it's rarer today that a single scene has more than one, though, so numAnimationsWeHave is likely usually 1.
	for ( int animationIndex = 0; animationIndex < numAnimationsInScene; animationIndex++ )
	{
		FbxAnimStack* anim = scene->GetSrcObject<FbxAnimStack>();
		if ( anim == nullptr )
			continue;

		//Get duration of this specific animation.
		FbxTime startTime = anim->LocalStart;
		FbxTime endTime = anim->LocalStop;
		FbxTime duration = endTime;
		if ( startTime > 0 )
			duration = endTime - startTime;
		
		//Will now evaluate using this FbxAnimStack set right here:
		scene->SetCurrentAnimationStack( anim );

		
		const char* motionName = anim->GetName(); //The actual name given to the motion in the file.
			//Use this to map to a name for next assignment.
			//Save it out using this name, most likely.

		float timespan = static_cast<float>( duration.GetSecondDouble() );

		//scanf from console command won't GetNextBool, so have to switch bool to uint32_t for fixed-size saving/loading.
		AnimationSequence* motion = new AnimationSequence( motionName, timespan, framerate, skeleton, ( useLocalOverGlobal ? 1 : 0 ) );

		int jointCount = skeleton->GetNumJoints();
		for ( int jointIndex = 0; jointIndex < jointCount; jointIndex++ )
		{
			FbxNode* node = jointToNodeMapping[ jointIndex ];

			// Extracting World (Model-space) Position
			// For Local, you would need to grab parent FbxNode* as well!

			FbxTime evalTime = FbxTime( 0 );

			Matrix4x4f* jointKeyframeBasePointer = motion->GetKeyframesForJoint( jointIndex );
			for ( uint32_t keyframeIndex = 0; keyframeIndex < motion->m_numKeyframesPerJoint; keyframeIndex++ )
			{
				Matrix4x4f* jointKeyframe = jointKeyframeBasePointer + keyframeIndex; //Pointer arithmetic.
				
				Matrix4x4f jointTransformDuringKeyframe( ROW_MAJOR );
				if ( useLocalOverGlobal )
					jointTransformDuringKeyframe = GetFbxNodeLocalJointSpaceTransformAtTime( node, evalTime, importBaseTransform );
				else
					jointTransformDuringKeyframe = GetFbxNodeGlobalModelSpaceTransformAtTime( node, evalTime, importBaseTransform );

				*jointKeyframe = jointTransformDuringKeyframe;

				evalTime += advance; //moves the "t" forwards in the FBX's evaluation.
			}
		}

		importedScene->m_animationSequences.push_back( motion );
	}
}


//--------------------------------------------------------------------------------------------------------------
static void ImportScene( ImportedScene* importedScene, FbxScene* scene, Matrix4x4Stack& matrixStack, float samplingFramerate, bool useLocalOverGlobal )
{
	//This function's very slow, the slowest point of the mesh import process. IsTriangleMesh() likely false without it.
	TriangulateScene( scene );

	FbxNode* rootNode = scene->GetRootNode();

	std::map< int, FbxNode* > jointToNodeMapping; //Could just be the index of the joint instead of the Joint*.
		//You fill it up inside ImportSkeletons and then index onto it in ImportMotions.
		//Or every time you add a joint, add an FbxNode* to a list that you pass between the two functions so you can index into both.
		//Usually your Joint won't + shouldn't have an FbxNode* on it--but you still need which FbxNode* it corresponds to while in the import process.

	ImportSceneSkeletons( importedScene, rootNode, matrixStack, nullptr, -1, jointToNodeMapping );
		//No skeleton yet, so nullptr and no parent, so -1.

	//Must come after ImportSceneSkeletons to have up-to-date node-joint mapping.
	ImportSceneMeshes( importedScene, rootNode, matrixStack, jointToNodeMapping );

	Matrix4x4f top = matrixStack.Peek(); //We only need the top matrix itself (engine, scene basis, scaling, etc), not the stack aspect.
	ImportMotions( importedScene, top, jointToNodeMapping, scene, samplingFramerate, useLocalOverGlobal );
		

	//Note we can add whatever we please to ImportScene in future that we'd like to import from an FBX scene, e.g. lights, cameras!
}


//--------------------------------------------------------------------------------------------------------------
static Matrix4x4f GetSceneBasis( FbxScene* scene )
{
	fbxsdk::FbxAxisSystem axisSystem = scene->GetGlobalSettings().GetAxisSystem(); //Has lots of info on how the FBX was stored.

	FbxAMatrix matrix; //A for affine, implicitly orthogonal.
	axisSystem.GetMatrix( matrix );

	return ToEngineMatrixFromFbxRowMajorMatrix( matrix, ROW_MAJOR );
}


//--------------------------------------------------------------------------------------------------------------
ImportedScene* FBXLoadSceneFromFile( const char* filename,
									 const Matrix4x4f& engineBasis,
									 bool isEngineBasisRightHanded,
									 const Matrix4x4f& transform /*=Matrix4x4::IDENTITY=*/, //e.g. scale down massive models, applied at very end (pushed first).
									 float samplingFramerate /*= 30.f*/,
									 bool useLocalOverGlobal /*= false*/ )
{
	UNREFERENCED( filename );

	FbxManager* fbxManager = FbxManager::Create();
	if ( nullptr == fbxManager )
	{
		DebuggerPrintf( "Could not create FBX manager." );
		return nullptr;
	}

	FbxIOSettings* io_settings = FbxIOSettings::Create( fbxManager, IOSROOT ); //Can provide names for things, but Forseth never uses it.
																			   //By default will load everything, so it's all we need to do.
																			   //To speed it up, can only load parts, but since this is an offline tool not a concern.
	fbxManager->SetIOSettings( io_settings );

	FbxImporter* importer = FbxImporter::Create( fbxManager, "" ); //Defines how the file we're reading is imported as a scene.

	std::string finalFilename = "Data/Models/";
	finalFilename += filename;
	bool loadSuccess = importer->Initialize( finalFilename.c_str(),
											 -1, // the file format - -1 lets it figure out for itself
											 fbxManager->GetIOSettings() // same as above pointer
											 );

	if ( loadSuccess )
	{
		FbxScene* scene = FbxScene::Create( fbxManager, "" );
		//Last argument is to name scene, makes sense upon generation, not on importing.

		bool importSuccess = importer->Import( scene ); //Converts above imported scene into the scene file.
		if ( importSuccess )
		{
			ImportedScene* importedScene = new ImportedScene();
			Matrix4x4Stack matrixStack( ROW_MAJOR );
//			FbxNode* root = scene->GetRootNode();

			matrixStack.Push( transform );
			matrixStack.Push( engineBasis );

			//Setup our initial transforms.
			Matrix4x4f sceneBasis = GetSceneBasis( scene );

			//Move from the scene basis to the FBX basis.
			Matrix4x4f sceneBasisInverse( ROW_MAJOR );
			sceneBasis.GetTranspose( sceneBasisInverse ); //Inverse is transpose for orthogonal matrix.
			
			//Handle if we're going between LHS and RHS.
			if ( !isEngineBasisRightHanded )
			{
				//Flip our engine's basis back and forward.
				Vector3f forwardK;
				sceneBasisInverse.GetBasisDirectionK( forwardK );
				//3rd row or col per ordering.
				sceneBasisInverse.SetBasisDirectionK( -forwardK );
			}

			matrixStack.Push( sceneBasisInverse );

			ImportScene( importedScene, scene, matrixStack, samplingFramerate, useLocalOverGlobal ); //First print current's attributes, then depth starting at 0, then all children.

			FBX_SAFE_DESTROY( scene );
			FBX_SAFE_DESTROY( importer );
			FBX_SAFE_DESTROY( io_settings );
			FBX_SAFE_DESTROY( fbxManager );

			return importedScene;
		}

		FBX_SAFE_DESTROY( scene );
	}
	else
	{
		g_theConsole->Printf( Stringf( "FBX SDK failed to import scene %s.", filename ).c_str() );
		g_theConsole->Printf( "Note code tries to load from Data/Models/ in active working directory, won't append .fbx." );
	}

	FBX_SAFE_DESTROY( importer );
	FBX_SAFE_DESTROY( io_settings );
	FBX_SAFE_DESTROY( fbxManager );

	return nullptr;
}


//--------------------------------------------------------------------------------------------------------------
void FBXList( Command& args ) //Convert to non-macro version if necessary, else see SD3 notes for what COMMAND translates to.
{
	std::string out;
	bool doesArgExist = args.GetNextString( &out, nullptr ); //Check m_argsList for another.
	if ( doesArgExist )
	{
		const char* filename = out.c_str();
		FBXListSceneFromFile( filename );
	}
	else
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: FBXList <filename>.fbx" );
	}
}


//--------------------------------------------------------------------------------------------------------------
MeshBuilder* CombineMeshBuildersInScene( ImportedScene* importedScene )
{
	MeshBuilder* currentBuilder = nullptr;

	std::vector<MeshBuilder*>& meshBuilders = importedScene->m_meshBuilders;
	std::sort( meshBuilders.begin(), meshBuilders.end(), //Sort by materials to ensure adjacent materials will be merged.
			   []( MeshBuilder* a, MeshBuilder* b ) { return a->GetMaterialID() < b->GetMaterialID(); } );
	std::sort( meshBuilders.begin(), meshBuilders.end(), //Sort by masks to ensure adjacent masks will be merged.
			   []( MeshBuilder* a, MeshBuilder* b ) { return a->GetCurrentDataMask() < b->GetCurrentDataMask(); } );

	for ( unsigned int builderIndex = 0; builderIndex < meshBuilders.size(); )
	{
		currentBuilder = new MeshBuilder( *meshBuilders[ builderIndex ] );

		unsigned int numBuildersAppendedThisIteration = currentBuilder->AppendMeshBuilderWhileMasksAndMaterialsMatch( std::vector<MeshBuilder*>( meshBuilders.begin() + builderIndex, meshBuilders.end() ) );

		g_theRenderer->AddModel( currentBuilder );

		g_theConsole->Printf( "Merged meshes %i through %i.", builderIndex, builderIndex + numBuildersAppendedThisIteration - 1 );

		builderIndex += numBuildersAppendedThisIteration;
	}

	return currentBuilder; //Result.
}

//--------------------------------------------------------------------------------------------------------------
void FBXLoad( Command& args )
{
	static unsigned int numInvocation = 0;

	std::string out;
	bool doesArgExist = args.GetNextString( &out, nullptr );
	if ( doesArgExist && ( out != "" ) )
	{
		//No way to read a bool with scanf, so:
		bool useLocalOverGlobal;
		int useLocalOverGlobalValue = 'M';
		args.GetNextInt( &useLocalOverGlobalValue, useLocalOverGlobalValue );
		useLocalOverGlobal = ( useLocalOverGlobalValue == 'L' );

		float samplingFramerate; //FPS to sample animation curve at.
		args.GetNextFloat( &samplingFramerate, 30.f );

		float scale; //e.g. .01f is 1/100 the size.
		args.GetNextFloat( &scale, 1.f );
		Matrix4x4f scalingTransform( ROW_MAJOR );
		scalingTransform.ClearToScaleMatrix( Vector3f( scale ) );

		const char* filename = out.c_str();

		Matrix4x4f outBasis = GetWorldChangeOfBasis( ROW_MAJOR );
		outBasis.SetToTranspose( false );
		ImportedScene* importedScene = FBXLoadSceneFromFile( filename, outBasis, true, scalingTransform, samplingFramerate, useLocalOverGlobal );

		if ( importedScene == nullptr )
		{
			g_theConsole->Printf( "Failed to load file. '%s'", filename );
			return;
		}
		else g_theConsole->Printf( "Loaded '%s'. Had %i meshes, %i skeletons, %i animations.", 
								   filename, 
								   importedScene->m_meshBuilders.size(),
								   importedScene->m_skeletons.size(),
								   importedScene->m_animationSequences.size() );

		//-----------------------------------------------------------------------------
		if ( g_lastLoadedMeshBuilder != nullptr )
		{
			delete g_lastLoadedMeshBuilder;
			g_lastLoadedMeshBuilder = nullptr;
		}
		g_lastLoadedMeshBuilder = CombineMeshBuildersInScene( importedScene );
		TODO( "Double-check the other combined builders are being cleaned up." );

		//--------------------------------------------------------------------------------------------------------------
// 		if ( g_lastLoadedSkeleton != nullptr ) 
// 		{
// 			delete g_lastLoadedSkeleton; //Handled by ~TheRenderer() now, so long as we add to it below.
// 			g_lastLoadedSkeleton = nullptr;
// 		}
		//g_lastLoadedSkeleton = new Skeleton(); //Unlike MeshBuilder above, we're just using the 1 pointer, so no need for new.

		for ( Skeleton* sk : importedScene->m_skeletons )
			g_theRenderer->AddSkeletonVisualization( sk );

		if ( g_theRenderer->GetNumAddedSkeletons() > 0 )
		{
			
			g_lastLoadedSkeleton = g_theRenderer->GetLastSkeletonAdded();

			//--//Initial Setting
			const int MAX_BONES = 200; //In total on skeleton.
			Matrix4x4f currentBoneTransforms[ MAX_BONES ];
			g_lastLoadedSkeleton->GetBoneMatrices( currentBoneTransforms, MAX_BONES );
			for ( auto& sp : *ShaderProgram::GetRegistry() )
				sp.second->SetMatrix4x4( "uBoneMatrices[0]", true, currentBoneTransforms, MAX_BONES );
			//		delete[] currentBoneTransforms;
		}

		//-----------------------------------------------------------------------------

		for ( AnimationSequence* as : importedScene->m_animationSequences )
			g_theRenderer->AddAnimationSequence( as );

		if ( g_theRenderer->GetNumAddedAnimations() > 0 )
		{
			g_lastLoadedAnimation = g_theRenderer->GetLastAnimationAdded();

			g_theConsole->Printf( "Use the AnimationPlay command to preview this model's animation." );
		}

		//-----------------------------------------------------------------------------
		delete importedScene;
		importedScene = nullptr;
	}
	else
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: FBXLoad <filename>.fbx [M/L=a Model/Local] [Anim Sampling FPS=30] [Scale=.01f]" );
	}
	numInvocation++;
}


//--------------------------------------------------------------------------------------------------------------
void MeshSaveLastMeshBuilderMade( Command& args )
{
	std::string outFilename;
	bool doesArgExist = args.GetNextString( &outFilename, nullptr );
	if ( doesArgExist && ( outFilename != "" ) )
	{
		if ( nullptr == g_lastLoadedMeshBuilder )
		{
			g_theConsole->Printf( "No MeshBuilder has been made and stored, use a command like FBXLoad/MeshSave first!" );
			return;
		}

		int outEndianMode;
		args.GetNextInt( &outEndianMode, 0 );

		const char* filename = outFilename.c_str();

		if ( g_lastLoadedMeshBuilder->WriteToFile( filename, false, outEndianMode ) )
			g_theConsole->Printf( "Write to file %s successful.", filename );
		else
			g_theConsole->Printf( "Write to file %s failed.", filename );
	}
	else
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: MeshSaveLastMeshBuilderMade <filename to save to, with extension> [0/1 = Little/Big-Endian]" );
	}
}


//--------------------------------------------------------------------------------------------------------------
void SkeletonSaveLastSkeletonMade( Command& args )
{
	if ( g_lastLoadedSkeleton == nullptr || g_lastLoadedSkeleton->m_hierarchyRoot == nullptr )
	{
		g_theConsole->Printf( "No skeleton to save, was one loaded or deleted?" );
		return;
	}

	std::string outFilename;
	bool doesArgExist = args.GetNextString( &outFilename, nullptr );
	if ( doesArgExist && ( outFilename != "" ) )
	{
		int outEndianMode;
		args.GetNextInt( &outEndianMode, 0 );

		const char* filename = outFilename.c_str();

		if ( g_lastLoadedSkeleton->WriteToFile( filename, false, outEndianMode ) )
			g_theConsole->Printf( "Write to file %s successful.", filename );
		else
			g_theConsole->Printf( "Write to file %s failed.", filename );
	}
	else
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SkeletonSaveLastSkeletonMade <filename to save to, with extension> [0/1 = Little/Big-Endian]" );
	}
}


//--------------------------------------------------------------------------------------------------------------
void AnimationSaveLastAnimationMade( Command& args )
{
	if ( g_lastLoadedAnimation == nullptr )
	{
		g_theConsole->Printf( "No animation to save, was one loaded or deleted?" );
		return;
	}

	std::string outFilename;
	bool doesArgExist = args.GetNextString( &outFilename, nullptr );
	if ( doesArgExist && ( outFilename != "" ) )
	{
		int outEndianMode;
		args.GetNextInt( &outEndianMode, 0 );

		const char* filename = outFilename.c_str();

		if ( g_lastLoadedAnimation->WriteToFile( filename, false, outEndianMode ) )
			g_theConsole->Printf( "Write to file %s successful.", filename );
		else
			g_theConsole->Printf( "Write to file %s failed.", filename );
	}
	else
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: AnimationSaveLastAnimationMade <filename to save to, with extension> [0/1 = Little/Big-Endian]" );
	}
}


//--------------------------------------------------------------------------------------------------------------
ImportedScene::~ImportedScene()
{
	//Walk a list and Destroy all meshes with FBX_SAFE_DESTROY!
	for ( MeshBuilder* mb : m_meshBuilders )
		delete mb;
}

#else // Same as !defined( TOOLS_BUILD ) would do. Defines all tool functions as empty, to compile them out.

void FBXListSceneFromFile( const char* ) {}
void FBXList( Command& ) {}
ImportedScene* FBXLoadSceneFromFile( const char* filename,
									 Matrix4x4f engineBasis,
									 bool isEngineBasisRightHanded,
									 Matrix4x4f transform /*=Matrix4x4::IDENTITY=*/ ) {
	UNREFERENCED( filename );
	UNREFERENCED( engineBasis );
	UNREFERENCED( isEngineBasisRightHanded );
	UNREFERENCED( transform );
	return nullptr;
}
void FBXLoad( Command& ) {}
void MeshSaveLastMeshBuilderMade( Command& ) {}
void SkeletonSaveLastSkeletonMade( Command& ) {}
void AnimationSaveLastAnimationMade( Command& ) {}

#endif

//--------------------------------------------------------------------------------------------------------------
void MeshLoadFromFile( Command& args )
{
	std::string outFilename;
	bool doesArgExist = args.GetNextString( &outFilename, nullptr );
	if ( doesArgExist && ( outFilename != "" ) )
	{
		int outEndianMode;
		args.GetNextInt( &outEndianMode, 0 );

		const char* filename = outFilename.c_str();
		g_lastLoadedMeshBuilder = new MeshBuilder();
		if ( g_lastLoadedMeshBuilder->ReadFromFile( filename, outEndianMode ) )
		{
			g_theConsole->Printf( "Read from file %s successful.", filename );
			g_theRenderer->AddModel( g_lastLoadedMeshBuilder );
		}
		else
		{
			g_theConsole->Printf( "Read from file %s failed, see log for details.", filename );
		}
	}
	else
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: MeshLoadFromFile <filename to load from, with extension> [0/1 = Little/Big-Endian]" );
	}
}


//--------------------------------------------------------------------------------------------------------------
void SkeletonLoadFromFile( Command& args )
{
	std::string outFilename;
	bool doesArgExist = args.GetNextString( &outFilename, nullptr );
	if ( doesArgExist && ( outFilename != "" ) )
	{
		int outEndianMode;
		args.GetNextInt( &outEndianMode, 0 );

		const char* filename = outFilename.c_str();
		g_lastLoadedSkeleton = new Skeleton();
		if ( g_lastLoadedSkeleton->ReadFromFile( filename, outEndianMode ) )
		{
			g_theConsole->Printf( "Read from file %s successful.", filename );
			g_theRenderer->AddSkeletonVisualization( g_lastLoadedSkeleton );
		}
		else
		{
			g_theConsole->Printf( "Read from file %s failed, see log for details.", filename );
		}
	}
	else
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SkeletonLoadFromFile <filename to load from, with extension> [0/1 = Little/Big-Endian]" );
	}
}


//--------------------------------------------------------------------------------------------------------------
void AnimationLoadFromFile( Command& args )
{
	std::string outFilename;
	bool doesArgExist = args.GetNextString( &outFilename, nullptr );
	if ( doesArgExist && ( outFilename != "" ) )
	{
		int outEndianMode;
		args.GetNextInt( &outEndianMode, 0 );

		const char* filename = outFilename.c_str();
		g_lastLoadedAnimation = new AnimationSequence();
		if ( g_lastLoadedAnimation->ReadFromFile( filename, outEndianMode ) )
		{
			g_theConsole->Printf( "Read from file %s successful.", filename );
			g_theRenderer->AddAnimationSequence( g_lastLoadedAnimation );
		}
		else
		{
			g_theConsole->Printf( "Read from file %s failed, see log for details.", filename );
		}
	}
	else
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: AnimationLoadFromFile <filename to load from, with extension> [0/1 = Little/Big-Endian]" );
	}
}