#pragma once

#include <string>
#include <map>
#include <memory>
#include "Engine/EngineCommon.hpp"
#include "Engine/Memory/UntrackedAllocator.hpp"


//--------------------------------------------------------------------------------------------------------------
class Mesh;
class Material;
struct Rgba;
class Sampler;
class Texture;
class VertexDefinition;


//--------------------------------------------------------------------------------------------------------------
class MeshRenderer;
typedef std::pair< std::string, MeshRenderer* > MeshRendererRegistryPair;
typedef std::map< std::string, MeshRenderer*, std::less<std::string>, UntrackedAllocator<MeshRendererRegistryPair> > MeshRendererRegistryMap;


//--------------------------------------------------------------------------------------------------------------
class MeshRenderer
{
public:
	TODO( "Make private! Was keeping visible for SD4 use." );
	MeshRenderer( std::shared_ptr<Mesh> mesh, Material* material );
	
	static MeshRenderer* CreateOrGetMeshRenderer( const std::string& meshRendererName, std::shared_ptr<Mesh> mesh, Material* material );
	static MeshRenderer* OverwriteMeshRenderer( const std::string& overwrittenMeshRendererName, std::shared_ptr<Mesh> mesh, Material* material );
	static void ClearMeshRenderers();
	static const MeshRendererRegistryMap* GetMeshRendererRegistry() { return &s_meshRendererRegistry; }
	~MeshRenderer();

	const std::string& GetMaterialName() const;
	std::shared_ptr<Mesh> GetMesh() const { return m_mesh; };
	const VertexDefinition* GetVertexDefinition() const;

	void SetMesh( std::shared_ptr<Mesh> mesh );
	void SetMaterial( Material* material, bool overwriteMemberMaterial );
	void SetMeshAndMaterial( std::shared_ptr<Mesh> mesh, Material* material ) { SetMesh(mesh); SetMaterial(material, true); }

	void Render();
	void Render( Mesh* mesh, Material* material );

	//Just calls the Material equivalents.
	void SetInt( const std::string& uniformNameVerbatim, int* newValue );
	void SetFloat( const std::string& uniformNameVerbatim, float* newValue );
	void SetVector2( const std::string& uniformNameVerbatim, const Vector2f* newValue );
	void SetVector3( const std::string& uniformNameVerbatim, const Vector3f* newValue );
	void SetVector4( const std::string& uniformNameVerbatim, const Vector4f* newValue );
	void SetMatrix4x4( const std::string& uniformNameVerbatim, bool shouldTranspose, const Matrix4x4f* newValue );
	void SetColor( const std::string& uniformNameVerbatim, const Rgba* newValue );
	void SetSampler( const std::string& uniformNameVerbatim, unsigned int newSamplerID );
	void SetTexture( const std::string& uniformNameVerbatim, unsigned int newTextureID );


private:
	void ResetVAO( Mesh* mesh, Material* material );
	static MeshRendererRegistryMap s_meshRendererRegistry;

	unsigned int m_vaoID;

	//Using shared pointers because multiple MeshRenderers may otherwise delete meshes/materials still in use.
	std::shared_ptr<Mesh> m_mesh;
	Material* m_material;
};
