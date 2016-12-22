#include "Game/Entities/Player.hpp"


#include "Game/Entities/Deflector.hpp"
#include "Engine/Input/TheInput.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Renderer/Particles/ParticleSystem.hpp"


//--------------------------------------------------------------------------------------------------------------
const float Player::s_PLAYER_MOVEMENT_SPEED = 6.f;


//--------------------------------------------------------------------------------------------------------------
Player::Player()
	: Agent( ENTITY_TYPE_PLAYER, "Player", PLAYER_START_POSITION, Rgba::CYAN )
	, m_isGeomancing( false )
	, m_aurameterValue( 0 )
	, m_smokeParticleSystem( nullptr )
{
	m_name = "Player";
}


//--------------------------------------------------------------------------------------------------------------
Player::~Player()
{
	//Deflector cleanup handled by TheGame::m_entities cleanup.

	if ( m_smokeParticleSystem != nullptr )
	{
		ParticleSystem::Destroy( m_smokeParticleSystem );
		m_smokeParticleSystem = nullptr;
	}
}


//--------------------------------------------------------------------------------------------------------------
void Player::Update( float deltaSeconds )
{
	if ( g_theInput->HasController() )
		UpdateFromXInput( deltaSeconds );
	else
		UpdateFromKeyboard( deltaSeconds );

	if ( m_aurameterValue >= MAX_AURAMETER )
		m_sprite->SetTint( Rgba::MAGENTA );
}


//--------------------------------------------------------------------------------------------------------------
Deflector* Player::AddDeflector( TerrainID currentTerrain )
{
	m_isGeomancing = false;
	m_aurameterValue = 0;
	m_sprite->SetTint( Rgba::WHITE );

	Deflector* newDeflector = Deflector::GetDeflectorCloneForTerrain( currentTerrain );
	newDeflector->SetPlayer( this );
	newDeflector->GetSprite()->SetLayerID( DEFLECTOR_LAYER_ID, "Deflectors" );
	newDeflector->GetSprite()->Enable();
	m_shields.push_back( newDeflector );

	return newDeflector;
}


//--------------------------------------------------------------------------------------------------------------
void Player::AddAurameterDelta( EntityType entityType )
{
	switch ( entityType )
	{
	case ENTITY_TYPE_ENEMY: m_aurameterValue += SCORE_FOR_KILLING_ENEMY; break;
	case ENTITY_TYPE_BULLET: m_aurameterValue += SCORE_FOR_DEFLECTING_BULLET; break;
	default: break;
	}
}


//--------------------------------------------------------------------------------------------------------------
void Player::UpdateFromXInput( float deltaSeconds )
{
	WorldCoords2D& playerPos = m_sprite->m_transform.m_position;
	float deltaMove = s_PLAYER_MOVEMENT_SPEED * deltaSeconds;

	if ( g_theInput->GetLeftStickPosition( CONTROLLER_ONE ).x > 0.f )
		playerPos += WorldCoords2D::UNIT_X * deltaMove;

	if ( g_theInput->GetLeftStickPosition( CONTROLLER_ONE ).x < 0.f )
		playerPos -= WorldCoords2D::UNIT_X * deltaMove;

	if ( g_theInput->GetLeftStickPosition( CONTROLLER_ONE ).y > 0.f )
		playerPos += WorldCoords2D::UNIT_Y * deltaMove;

	if ( g_theInput->GetLeftStickPosition( CONTROLLER_ONE ).y < 0.f )
		playerPos -= WorldCoords2D::UNIT_Y * deltaMove;
}


//--------------------------------------------------------------------------------------------------------------
void Player::UpdateFromKeyboard( float deltaSeconds )
{
	WorldCoords2D& playerPos = m_sprite->m_transform.m_position;
	float deltaMove = s_PLAYER_MOVEMENT_SPEED * deltaSeconds;

	if ( g_theInput->IsKeyDown( KEY_TO_MOVE_RIGHT_2D ) )
		playerPos += WorldCoords2D::UNIT_X * deltaMove;

	if ( g_theInput->IsKeyDown( KEY_TO_MOVE_LEFT_2D ) )
		playerPos -= WorldCoords2D::UNIT_X * deltaMove;

	if ( g_theInput->IsKeyDown( KEY_TO_MOVE_UP_2D ) )
		playerPos += WorldCoords2D::UNIT_Y * deltaMove;

	if ( g_theInput->IsKeyDown( KEY_TO_MOVE_DOWN_2D ) )
		playerPos -= WorldCoords2D::UNIT_Y * deltaMove;

	if ( g_theInput->WasKeyPressedOnce( KEY_TO_SPEND_AURAMETER ) && m_aurameterValue >= MAX_AURAMETER )
		m_isGeomancing = true;

	if ( g_theInput->WasKeyPressedOnce( 'F' ) )
		m_aurameterValue += 10;

	if ( g_theInput->WasKeyPressedOnce( KEY_TO_THROW_DEFLECTOR ) && m_shields.size() > 0 )
	{
		m_shields.back()->Launch();
		m_shields.pop_back();
	}
}
