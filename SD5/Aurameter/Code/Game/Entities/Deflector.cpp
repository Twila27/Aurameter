#include "Game/Entities/Deflector.hpp"

#include "Game/Entities/Player.hpp"
#include "Engine/Renderer/SpriteRenderer.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/PhysicsUtils.hpp"
#include "Engine/Physics/Forces.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC const Vector2f Deflector::s_MAX_RADIUS = Vector2f( 5.f, 5.f );
STATIC const float Deflector::s_EXPANSION_RATE = 2.5f;
STATIC DeflectorRegistryMap Deflector::s_deflectorRegistry;


//--------------------------------------------------------------------------------------------------------------
static void DeflectLogic_Tundra( LinearDynamicsState* deflectedBulletState )
{
	deflectedBulletState->SetVelocity( deflectedBulletState->GetVelocity() * -5.f );
}
static void DeflectLogic_Volcano( LinearDynamicsState* deflectedBulletState )
{
	deflectedBulletState->SetPosition( Vector3f( deflectedBulletState->GetPosition().x, SpriteRenderer::GetVirtualScreenHeight().y, 0.f ) );
//	deflectedBulletState->AddForce( new SpringForce( 500.f, Vector3f( 0.f, 1.f, 0.f ), .5f ) );
}
static void DeflectLogic_Desert( LinearDynamicsState* deflectedBulletState )
{
	deflectedBulletState->SetVelocity( Vector3f::ZERO );
	deflectedBulletState->AddForce( new ConstantWindForce( 90.81f, Vector3f( GetRandomFloatInRange( -1.f, 1.f ), GetRandomFloatInRange( -1.f, 1.f ), 0.f ) ) );
}
static void DeflectLogic_Swamp( LinearDynamicsState* deflectedBulletState )
{
	deflectedBulletState->SetVelocity( deflectedBulletState->GetVelocity() * -5.f );
//	deflectedBulletState->AddForce( new DebrisForce( 90.81f ) );
}
static void DeflectLogic_Space( LinearDynamicsState* deflectedBulletState )
{
	deflectedBulletState->AddForce( new WormholeForce( Vector3f::ZERO, 9.81f, WORLD3D_UP ) );
}


//--------------------------------------------------------------------------------------------------------------
STATIC void Deflector::RegisterDeflector( const std::string& deflectorName, const TerrainID& terrain, const Rgba& tint, DeflectorCallback* deflectLogic )
{
	Deflector* templateDeflector = new Deflector( deflectorName, deflectLogic, nullptr );
	
	templateDeflector->m_sprite->SetTint( tint );

	s_deflectorRegistry[ terrain ] = templateDeflector;
}


//--------------------------------------------------------------------------------------------------------------
STATIC void Deflector::LoadAllDeflectors() //Would need reflection (or mimicry of it) to actually allow it to be data-driven.
{

	RegisterDeflector( "Tundra Deflector", "Frozen Earth", Rgba::CYAN, DeflectLogic_Tundra );
	RegisterDeflector( "Volcano Deflector", "Molten Earth", Rgba::RED, DeflectLogic_Volcano);
	RegisterDeflector( "Desert Deflector", "Desert Earth", Rgba::YELLOW, DeflectLogic_Desert);
	RegisterDeflector( "Swamp Deflector", "Swampy Earth", Rgba::GREEN * Rgba::GRAY, DeflectLogic_Swamp );
	RegisterDeflector( "Space Deflector", "Another Earth", Rgba::MAGENTA, DeflectLogic_Space );
}


//--------------------------------------------------------------------------------------------------------------
std::string Deflector::GetDeflectorDescForTerrain( TerrainID terrain )
{
	if ( terrain == "Desert Earth" )
		return "Random Winds";
	if ( terrain == "Molten Earth" )
		return "Erupt into the Sky";
	if ( terrain == "Another Earth" )
		return "Wormhole";
	else //Frozen, Swampy Earth
		return "Reflecting Mirrors";

}


//--------------------------------------------------------------------------------------------------------------
STATIC void Deflector::CleanupRegistry()
{
	for ( const DeflectorRegistryPair& pair : s_deflectorRegistry )
		delete pair.second;
	s_deflectorRegistry.clear();
}


//--------------------------------------------------------------------------------------------------------------
Deflector::Deflector( const std::string& name, DeflectorCallback* cb, const Player* player )
	: GameEntity( ENTITY_TYPE_DEFLECTOR, "Deflector", WorldCoords2D::ZERO )
	, m_player( player )
	, m_hasLaunched( false )
	, m_deflectLogic( cb )
{
	m_name = name;
}


//--------------------------------------------------------------------------------------------------------------
Deflector::Deflector( const Deflector& other ) 
	: GameEntity( other )
	, m_hasLaunched( other.m_hasLaunched )
	, m_player( other.m_player )
	, m_deflectLogic( other.m_deflectLogic )
{
}


//--------------------------------------------------------------------------------------------------------------
void Deflector::Update( float deltaSeconds )
{
	if ( m_hasLaunched )
	{
		m_sprite->m_transform.m_scale += deltaSeconds * s_EXPANSION_RATE;
		m_sprite->m_transform.m_rotationAngleDegrees += deltaSeconds * s_EXPANSION_RATE;
		GameEntity::Update( deltaSeconds );
	}
	else
	{
		SetPosition( m_player->GetPosition() );
	}
}


//--------------------------------------------------------------------------------------------------------------
void Deflector::Launch()
{
	//Should we add a velocity?
	m_hasLaunched = true;
}


//--------------------------------------------------------------------------------------------------------------
void Deflector::SetPlayer( const Player* player )
{
	m_player = player;
	SetPosition( m_player->GetPosition() );
}


//--------------------------------------------------------------------------------------------------------------
bool Deflector::IsExpired()
{
	return m_sprite->m_transform.m_scale > s_MAX_RADIUS;
}
