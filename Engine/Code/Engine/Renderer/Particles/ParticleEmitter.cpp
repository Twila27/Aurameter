#include "Engine/Renderer/Particles/ParticleEmitter.hpp"
#include "Engine/Renderer/Particles/ParticleEmitterDefinition.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/PhysicsUtils.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Renderer/Vertexes.hpp"
#include "Engine/Renderer/Sprite.hpp"


//--------------------------------------------------------------------------------------------------------------
const unsigned int MAX_BATCH_SIZE = 5000;


//--------------------------------------------------------------------------------------------------------------
STATIC ParticleEmitter* ParticleEmitter::Create( ParticleEmitterDefinition* emitterDefn, const Vector2f& systemPosition )
{
	static int numInvocation = 0;

	ParticleEmitter* emitter = new ParticleEmitter();
	emitter->m_emitterDefinition = emitterDefn;
	emitter->m_position = systemPosition;
	emitter->m_numInitialSpawnsLeft = emitterDefn->m_initialSpawnCount;
	Material* emitterMaterial = Material::CreateOrGetMaterial( Stringf( "%s%d_Material", emitterDefn->GetName().c_str(), numInvocation ), emitterDefn->GetRenderState(), &Vertex2D_PCT::DEFINITION, "BasicSprite" );
	emitter->m_emitterMeshRenderer->SetMaterial( emitterMaterial, true );
	return emitter;
}


//--------------------------------------------------------------------------------------------------------------
bool ParticleEmitter::IsLooping() const
{
	return m_emitterDefinition->IsLooping();
}


//--------------------------------------------------------------------------------------------------------------
bool ParticleEmitter::IsExpired() const
{
	bool hasNoParticles = ( m_particles.size() == 0 );
	bool meetsLoopingCase = m_isMarkedForDeletion && hasNoParticles;
	bool meetsNonLoopingCase = hasNoParticles && ( m_emitterDefinition->m_secondsPerSpawn == 0.f ) && ( m_numInitialSpawnsLeft == 0 );
	
	return meetsLoopingCase || meetsNonLoopingCase;
}


//--------------------------------------------------------------------------------------------------------------
void ParticleEmitter::MarkForDeletion()
{
	m_isMarkedForDeletion = true;
}


//--------------------------------------------------------------------------------------------------------------
void ParticleEmitter::Update( float deltaSeconds )
{

	//KEY: do not update after spawn, could create and immediately die (very important).
	m_emitterDefinition->Update( m_particles, deltaSeconds );

	//Destroy before create to free up memory before we allocate more (minor).
	m_emitterDefinition->Destroy( m_particles );

	if ( m_isMarkedForDeletion )
		return;

	m_secondsSinceLastSpawn += deltaSeconds;
	if ( m_numInitialSpawnsLeft > 0 || IsLooping() )
	{
		int numSpawns = m_emitterDefinition->Spawn( m_particles, m_position, m_secondsSinceLastSpawn );
		if ( m_numInitialSpawnsLeft > 0 ) //To keep from going negative and underflowing for looping systems.
			m_numInitialSpawnsLeft -= numSpawns;
	}
}


//--------------------------------------------------------------------------------------------------------------
void ParticleEmitter::Render()
{
	size_t index = 0;
	while ( index < m_particles.size() )
	{
		index += FillMesh( &m_particles[ index ], GetMin( MAX_BATCH_SIZE, m_particles.size() - index ) ); //Return # copied.

		m_emitterMeshRenderer->Render();
	}
	TODO( "The next step's to have a m_mesh2 that you pingpong double-buffering with m_mesh1 above." );
}


//--------------------------------------------------------------------------------------------------------------
ParticleEmitter::ParticleEmitter()
	: m_isMarkedForDeletion( false )
	, m_emitterDefinition( nullptr )
	, m_numInitialSpawnsLeft( 0 )
	, m_emitterMeshRenderer( new MeshRenderer( m_particleBatchMesh, nullptr ) )
	, m_particleBatchMesh( std::shared_ptr<Mesh>( new Mesh( BufferUsage::STATIC_DRAW, Vertex2D_PCT::DEFINITION, 0, nullptr, 0, nullptr, 0, nullptr ) ) )
	, m_secondsSinceLastSpawn( 0.f )
{
}


//--------------------------------------------------------------------------------------------------------------
ParticleEmitter::~ParticleEmitter()
{
	delete m_emitterMeshRenderer;
}


//--------------------------------------------------------------------------------------------------------------
int ParticleEmitter::FillMesh( Particle* baseAddress, unsigned int numToAdd )
{
	//Order taken from CreateQuadMesh used in SpriteRenderer::Startup, to let us keep the same IBO.
	std::vector< Vertex2D_PCT > vertices;
	std::vector< unsigned int > indices;
	m_particleBatchMesh->ClearDrawInstructions();
	for ( size_t particleIndex = 0; particleIndex < numToAdd; particleIndex++ )
	{
		Particle* currentParticle = baseAddress + particleIndex;

		Sprite* sprite = currentParticle->m_sprite;
		sprite->m_transform.m_position = currentParticle->m_physicsInfo->GetPosition().xy();
		sprite->m_transform.m_scale = currentParticle->m_scale;


		AABB2f worldBounds = sprite->GetVirtualBoundsInWorld();

		vertices.push_back( Vertex2D_PCT( Vector2f( worldBounds.mins.x, worldBounds.maxs.y ),	currentParticle->m_tint,	Vector2f( 0.f, 0.f ) ) ); //Top-left.		//0
		vertices.push_back( Vertex2D_PCT( Vector2f( worldBounds.maxs.x, worldBounds.mins.y ),	currentParticle->m_tint,	Vector2f( 1.f, 1.f ) ) ); //Bottom-right.	//1
		vertices.push_back( Vertex2D_PCT( worldBounds.mins,										currentParticle->m_tint,	Vector2f( 0.f, 1.f ) ) ); //Bottom-left.	//2
		vertices.push_back( Vertex2D_PCT( worldBounds.maxs,										currentParticle->m_tint,	Vector2f( 1.f, 0.f ) ) ); //Top-right.	//3

		indices.push_back( 2 );
		indices.push_back( 1 );
		indices.push_back( 0 );
		indices.push_back( 0 );
		indices.push_back( 1 );
		indices.push_back( 3 ); //Counter-Clockwise, else renders to the back and the quad won't show with backface culling.
	}

	m_particleBatchMesh->AddDrawInstruction( VertexGroupingRule::AS_TRIANGLES, 0, indices.size(), true );
	ASSERT_OR_DIE( numToAdd * 4 == vertices.size(), nullptr );
	m_particleBatchMesh->SetThenUpdateMeshBuffers( vertices.size(), (void*)vertices.data(), indices.size(), (void*)indices.data() );


	return numToAdd;
}
