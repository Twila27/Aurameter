#include "Game/Entities/GameEntity.hpp"


#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Physics/PhysicsUtils.hpp"
#include "Engine/Physics/Forces.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC const EntityID GameEntity::s_INVALID_ID = -1;
STATIC EntityID GameEntity::s_BASE_ENTITY_ID = 1; //Not zero, since then it might be confused for having no value.


//--------------------------------------------------------------------------------------------------------------
GameEntity::GameEntity( EntityType entityType )
	: GameEntity( entityType, "Default", WorldCoords2D::ONE )
{
}



//--------------------------------------------------------------------------------------------------------------
GameEntity::GameEntity( EntityType entityType, ResourceID resName, const WorldCoords2D& pivotPosition, const Rgba& tint /*= Rgba::WHITE */ )
	: Entity()
	, m_maxHealth( GetHealthForName( resName ) )
	, m_hasBeenDeflected( false )
	, m_entityType( entityType )
	, m_entityID( s_BASE_ENTITY_ID++ )
	, m_sprite( Sprite::Create( resName, pivotPosition, tint ) )
	, m_name( resName )
	, m_rigidbody( new LinearDynamicsState( WorldCoords3D( pivotPosition.x, pivotPosition.y, 0.f ) ) )
{
	m_health = m_maxHealth;
}


//--------------------------------------------------------------------------------------------------------------
GameEntity::GameEntity( const GameEntity& other )
	: Entity( other )
	, m_health( other.m_health )
	, m_maxHealth( other.m_maxHealth )
	, m_hasBeenDeflected( other.m_hasBeenDeflected )
	, m_entityType( other.m_entityType )
	, m_entityID( s_BASE_ENTITY_ID++ )
	, m_name( other.m_name )
	, m_sprite( Sprite::Create( other.m_sprite ) )
	, m_rigidbody( new LinearDynamicsState( WorldCoords3D( other.m_sprite->GetPivotSpriteRelative().x, other.m_sprite->GetPivotSpriteRelative().y, 0.f ) ) )
{
	std::vector<Force*> forces;
	other.m_rigidbody->GetForces( forces );
	for ( Force* f : forces )
		m_rigidbody->AddForce( f->GetCopy() );
}


//--------------------------------------------------------------------------------------------------------------
GameEntity::~GameEntity()
{
	delete m_sprite;
	delete m_rigidbody;
}


//--------------------------------------------------------------------------------------------------------------
void GameEntity::PopulateFromXMLNode( const XMLNode& entityNode )
{
	m_name = ReadXMLAttribute( entityNode, "name", m_name );

	std::string spriteResName;
	spriteResName = ReadXMLAttribute( entityNode, "resourceName", spriteResName );
	m_sprite = Sprite::Create( spriteResName, WorldCoords2D::ZERO );
}


//--------------------------------------------------------------------------------------------------------------
WorldCoords2D GameEntity::GetPosition() const
{
	return m_sprite->m_transform.m_position;
}


//--------------------------------------------------------------------------------------------------------------
void GameEntity::Enable()
{
	m_sprite->Enable();
}


//--------------------------------------------------------------------------------------------------------------
void GameEntity::SetPosition( const WorldCoords2D& newPosition )
{
	m_sprite->m_transform.m_position = newPosition;
	m_rigidbody->SetPosition( Vector3f( newPosition.x, newPosition.y, 0.f ) );
}


//--------------------------------------------------------------------------------------------------------------
void GameEntity::SetMaterial( Material* newMat )
{
	m_sprite->SetMaterial( newMat );
}


//--------------------------------------------------------------------------------------------------------------
void GameEntity::Update( float deltaSeconds )
{
	m_rigidbody->StepWithForwardEuler( 1.f, deltaSeconds );
	m_sprite->m_transform.m_position = m_rigidbody->GetPosition().xy();
}


//--------------------------------------------------------------------------------------------------------------
bool GameEntity::AddHealthDelta( int delta )
{
	bool didHeal = true;

	m_health += delta;
	if ( m_health > m_maxHealth )
	{
		m_health = m_maxHealth;
		didHeal = false;
	}

	return didHeal;
}


//--------------------------------------------------------------------------------------------------------------
bool GameEntity::SubtractHealthDelta( int delta )
{
	bool didHeal = true;

	m_health -= delta;
	if ( m_health < 0 )
	{
		m_health = 0;
		didHeal = false;
	}

	return didHeal;
}
