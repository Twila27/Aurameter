#include "Game/Entities/Agent.hpp"


//--------------------------------------------------------------------------------------------------------------
Agent::Agent( EntityType entityType )
	: GameEntity( entityType, "Default", WorldCoords2D::ZERO )
{

}


//--------------------------------------------------------------------------------------------------------------
Agent::Agent( EntityType entityType, ResourceID spriteName, const WorldCoords2D& pivotPosition, const Rgba& tint /*= Rgba::WHITE */ )
	: GameEntity( entityType, spriteName, pivotPosition, tint )
{

}
