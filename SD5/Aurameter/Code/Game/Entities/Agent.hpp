#pragma once


#include "Game/Entities/GameEntity.hpp"
#include "Engine/Renderer/Rgba.hpp"


//-----------------------------------------------------------------------------
struct XMLNode;


//-----------------------------------------------------------------------------
class Agent : public GameEntity
{
public:
	Agent( EntityType entityType );
	Agent( EntityType entityType, ResourceID spriteName, const WorldCoords2D& pivotPosition, const Rgba& tint = Rgba::WHITE );
	

protected:
	void PopulateFromXMLNode( const XMLNode& agentNode ) { GameEntity::PopulateFromXMLNode( agentNode ); }
};
