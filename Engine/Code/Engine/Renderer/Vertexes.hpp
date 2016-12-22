#pragma once


#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/Rgba.hpp"
#include "Engine/Renderer/VertexDefinition.hpp"


struct Vertex3D_Superset
{
	Vertex3D_Superset() : m_boneWeights( DEFAULT_BONE_WEIGHTS ), m_jointIndices( DEFAULT_JOINT_INDICES ) {}

	Vector3f m_position;

	Rgba m_color;

	Vector2f m_texCoords0;
	Vector2f m_texCoords1;
	Vector2f m_texCoords2;
	Vector2f m_texCoords3;

	Vector3f m_tangent;
	Vector3f m_bitangent;
	Vector3f m_normal;

	//Limiting # joints that can affect a vertex to 4 by using Vector4.
		//An artist in using auto-weights or a weight brush can be imprecise and associate 4+, 
		//but generally most of those are negligibly small, so we'll just take the 4 largest weights.
	Vector4f m_boneWeights; //The bone space defined by a joint. Components x+y+z+w must sum to one.
		//e.g. the default 1,0,0,0 -> receive full weight (the 1 in the .x)
		//note -1 would go opposite a joint's pivot direction, hence always needs to be positive.
			//(it would then be unsigned floats, but that doesn't exist...)
	Vector4<unsigned int> m_jointIndices; //Maps above boneWeights to their joints in a skeleton.
		//e.g. the default 0,0,0,0 -> of bone 0, the root (the first 0 in the .x)

	static const VertexDefinition DEFINITION;
	static const Vector4f DEFAULT_BONE_WEIGHTS;
	static const Vector4<unsigned int> DEFAULT_JOINT_INDICES;


private:

	static const VertexAttribute ATTRIBUTES[];
};


struct Vertex2D_PCT
{
public:

	Vector2f m_position;
	Rgba m_color;
	Vector2f m_texCoords;

	Vertex2D_PCT();
	Vertex2D_PCT( const Vector2f& position, const Vector2f& texCoords = Vector2f::ZERO, const Rgba& color = Rgba::WHITE );
	Vertex2D_PCT( const Vector2f& position, const Rgba& color = Rgba::WHITE, const Vector2f& texCoords = Vector2f::ZERO );

	static const VertexDefinition DEFINITION;


private:

	static const VertexAttribute ATTRIBUTES[];
};


struct Vertex3D_PCT
{
public:

	Vector3f m_position;
	Rgba m_color;
	Vector2f m_texCoords;

	Vertex3D_PCT();
	Vertex3D_PCT( const Vector3f& position, const Vector2f& texCoords = Vector2f::ZERO, const Rgba& color = Rgba::WHITE );
	Vertex3D_PCT( const Vector3f& position, const Rgba& color = Rgba::WHITE, const Vector2f& texCoords = Vector2f::ZERO );

	static const VertexDefinition DEFINITION;


private:

	static const VertexAttribute ATTRIBUTES[];
};


struct Vertex3D_PCTWI
{
public:

	Vector3f m_position;
	Rgba m_color;
	Vector2f m_texCoords;
	
	Vertex3D_PCTWI() : m_boneWeights( DEFAULT_BONE_WEIGHTS ), m_jointIndices( DEFAULT_JOINT_INDICES ) {}
	//Limiting # joints that can affect a vertex to 4 by using Vector4.
		//An artist in using auto-weights or a weight brush can be imprecise and associate 4+, 
		//but generally most of those are negligibly small, so we'll just take the 4 largest weights.
	Vector4f m_boneWeights; //The bone space defined by a joint. Components x+y+z+w must sum to one.
		//e.g. the default 1,0,0,0 -> receive full weight (the 1 in the .x)
		//note -1 would go opposite a joint's pivot direction, hence always needs to be positive.
			//(it would then be unsigned floats, but that doesn't exist...)
	Vector4<unsigned int> m_jointIndices; //Maps above boneWeights to their joints in a skeleton.
		//e.g. the default 0,0,0,0 -> of bone 0, the root (the first 0 in the .x)

	static const VertexDefinition DEFINITION;
	static const Vector4f DEFAULT_BONE_WEIGHTS;
	static const Vector4<unsigned int> DEFAULT_JOINT_INDICES;


private:

	static const VertexAttribute ATTRIBUTES[];
};


struct Vertex3D_PCUTB
{
public:

	Vector3f m_position;
	Rgba m_color;
	Vector2f m_texCoords;
	Vector3f m_tangent; //Along u-direction (1,0), think derivative/rate of change of texture coordinates over mesh surface.
	Vector3f m_bitangent; //Along v-direction (0,1). But in OpenGL due to the flip this is really -v. Just vertical in general.

	Vertex3D_PCUTB();
	Vertex3D_PCUTB( const Vector3f& position, const Vector3f& tangent, const Vector3f& bitangent, const Vector2f& texCoords = Vector2f::ZERO, const Rgba& color = Rgba::WHITE );
	Vertex3D_PCUTB( const Vector3f& position, const Vector3f& tangent, const Vector3f& bitangent, const Rgba& color = Rgba::WHITE, const Vector2f& texCoords = Vector2f::ZERO );

	static const VertexDefinition DEFINITION;


private:

	static const VertexAttribute ATTRIBUTES[];
};


typedef void( VertexCopyCallback )( void* dest, const Vertex3D_Superset& src );
VertexCopyCallback* GetCopyFunctionForVertexDefinition( const VertexDefinition* vdefn );
void CopyToVertex3D_PCT( void* dest, const Vertex3D_Superset& src );
void CopyToVertex3D_PCUTB( void* dest, const Vertex3D_Superset& src );
void CopyToVertex3D_Superset( void* dest, const Vertex3D_Superset& src );