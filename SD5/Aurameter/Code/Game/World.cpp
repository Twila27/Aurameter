#include "Game/World.hpp"

#include "Engine/FileUtils/FileUtils.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"
#include "Engine/Renderer/AnimatedSprite.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Renderer/SpriteRenderer.hpp"
#include "Engine/Core/EngineEvent.hpp"
#include "Engine/Core/TheEventSystem.hpp"

#include "Game/Factories/Pattern.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC ArenaRegistryVector World::s_arenaTemplateRegistry;


//--------------------------------------------------------------------------------------------------------------
static void GetArenaNameFromXML( const char* xmlFilename, const XMLNode& arenaNode, std::string& out_name )
{
	char arenaName[ 80 ];
	sscanf_s( xmlFilename, "Data/XML/Arenas/%[^.].Arena.xml", &arenaName, _countof( arenaName ) );

	std::string nameInsideFile;
	nameInsideFile = ReadXMLAttribute( arenaNode, "name", nameInsideFile );

	if ( nameInsideFile != "" )
		out_name = nameInsideFile;
	else
		out_name = arenaName;
}


//--------------------------------------------------------------------------------------------------------------
STATIC void World::LoadAllArenas()
{
	//Note we may have more than one Arena in a file.
	std::vector< std::string > m_arenaFiles = EnumerateFilesInDirectory( "Data/XML/Arenas", "*.Arena.xml" );

	for ( unsigned int arenaFileIndex = 0; arenaFileIndex < m_arenaFiles.size(); arenaFileIndex++ )
	{
		const char* xmlFilename = m_arenaFiles[ arenaFileIndex ].c_str();
		XMLNode arenasRoot = XMLNode::openFileHelper( xmlFilename, "Arenas" );

		for ( int arenaIndex = 0; arenaIndex < arenasRoot.nChildNode(); arenaIndex++ )
		{
			XMLNode arenaNode = arenasRoot.getChildNode( arenaIndex );

			World* newArenaTemplate = new World( arenaNode );
			GetArenaNameFromXML( xmlFilename, arenaNode, newArenaTemplate->m_terrain );
			s_arenaTemplateRegistry.push_back( newArenaTemplate );
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
STATIC void World::CloneRegistry( std::vector< World* >& outArenas )
{
	for ( World* arena : s_arenaTemplateRegistry )
	{
		outArenas.push_back( new World( *arena ) );
	}
}


//--------------------------------------------------------------------------------------------------------------
STATIC void World::CleanupRegistry()
{
	for ( World* arenaTemplate : s_arenaTemplateRegistry )
		delete arenaTemplate;

	s_arenaTemplateRegistry.clear();
}

//--------------------------------------------------------------------------------------------------------------
World::World( const World& other )
	: m_terrain( other.m_terrain )
	, m_background( Sprite::Create( other.m_background ) )
	, m_transition( AnimatedSprite::Create( dynamic_cast<AnimatedSprite*>( other.m_transition ) ) )
	, m_titleCard( Sprite::Create( other.m_titleCard ) )
	, m_currentWave( other.m_currentWave )
{
	m_background->SetLayerID( other.m_background->GetLayerID() );

	if ( m_transition != nullptr )
		m_transition->SetLayerID( other.m_transition->GetLayerID() );

	if ( m_titleCard != nullptr )
		m_titleCard->SetLayerID( other.m_titleCard->GetLayerID() );

	for ( SpawnPattern* formation : other.m_waves )
		m_waves.push_back( new SpawnPattern( *formation ) );
}


//--------------------------------------------------------------------------------------------------------------
World::~World()
{
	delete m_background;
	m_background = nullptr;
	delete m_transition;
	m_transition = nullptr;
	delete m_titleCard;
	m_titleCard = nullptr;
}


//--------------------------------------------------------------------------------------------------------------
void World::PopulateFromXMLNode( const XMLNode& arenaNode )
{
	std::string backgroundResourceName;
	backgroundResourceName = ReadXMLAttribute( arenaNode, "background", backgroundResourceName );

	std::string titleCardResourceName;
	titleCardResourceName = ReadXMLAttribute( arenaNode, "titleCard", titleCardResourceName );

	//Replace by an XML load-in like the above if we get to per-world transitions in polish!
	m_transition = AnimatedSprite::Create( "WorldTransition", WorldCoords2D::ZERO );
	m_transition->SetLayerID( WORLD_TRANSITION_LAYER_ID, "WorldTransition" );

	m_background = Sprite::Create( backgroundResourceName, WorldCoords2D::ZERO );
	m_background->SetLayerID( BACKGROUND_LAYER_ID, "Background" );

	m_titleCard = Sprite::Create( titleCardResourceName, TITLE_CARD_WORLD_POSITION );
	m_titleCard->SetLayerID( TITLE_CARD_LAYER_ID, "Title Card" );
	SpriteRenderer::SetLayerVirtualSize( TITLE_CARD_LAYER_ID, 5 );

	int numWaves = arenaNode.nChildNode( "Pattern" );
	m_waves.resize( numWaves );

	for ( int waveIndex = 0; waveIndex < numWaves; waveIndex++ )
	{
		XMLNode patternNode = arenaNode.getChildNode( "Pattern" );

		std::string patternName;
		patternName = ReadXMLAttribute( patternNode, "name", patternName );

		m_waves[ waveIndex ] = SpawnPattern::CreateOrGetPattern( patternName, patternNode );
	}
}


//--------------------------------------------------------------------------------------------------------------
int World::SpawnNextWave( std::vector<GameEntity*>& outEntityList, const WorldCoords2D& offsetFromOrigin /*= WorldCoords2D::ZERO*/ )
{
	SpawnPattern* pattern = m_waves[ m_currentWave++ ]; 
	pattern->Instantiate( offsetFromOrigin, outEntityList );
	return pattern->m_numSpawns;
}


//--------------------------------------------------------------------------------------------------------------
bool World::FinishTransition( EngineEvent* )
{
	m_transition->Disable();
	m_titleCard->Disable();

	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool World::HideBackground( EngineEvent* )
{
	m_background->Disable();

	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool World::ShowBackground( EngineEvent* )
{
	m_background->Enable();

	return true;
}
