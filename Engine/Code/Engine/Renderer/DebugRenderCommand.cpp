#include "Engine/Renderer/DebugRenderCommand.hpp"

#include "Engine/Renderer/TheRenderer.hpp"


//--------------------------------------------------------------------------------------------------------------
std::list< DebugRenderCommand* >* g_theDebugRenderCommands = nullptr;


//--------------------------------------------------------------------------------------------------------------
void DebugRenderCommandPoint::Render()
{
	//x in XY.
	g_theRenderer->DrawLine( m_position + ( Vector3f( -1.f, -1.f, 0.f ) * m_sizeScalar ), m_position + ( Vector3f( 1.f, 1.f, 0.f ) * m_sizeScalar ), m_color, m_color, m_lineThickness );
	g_theRenderer->DrawLine( m_position + ( Vector3f( -1.f, 1.f, 0.f ) * m_sizeScalar ), m_position + ( Vector3f( 1.f, -1.f, 0.f ) * m_sizeScalar ), m_color, m_color, m_lineThickness );

	//x in XZ.
	g_theRenderer->DrawLine( m_position + ( Vector3f( -1.f, 0.f, -1.f ) * m_sizeScalar ), m_position + ( Vector3f( 1.f, 0.f, 1.f ) * m_sizeScalar ), m_color, m_color, m_lineThickness );
	g_theRenderer->DrawLine( m_position + ( Vector3f( -1.f, 0.f, 1.f ) * m_sizeScalar ), m_position + ( Vector3f( 1.f, 0.f, -1.f ) * m_sizeScalar ), m_color, m_color, m_lineThickness );

	//x in YZ.
	g_theRenderer->DrawLine( m_position + ( Vector3f( 0.f, -1.f, -1.f ) * m_sizeScalar ), m_position + ( Vector3f( 0.f, 1.f, 1.f ) * m_sizeScalar ), m_color, m_color, m_lineThickness );
	g_theRenderer->DrawLine( m_position + ( Vector3f( 0.f, -1.f, 1.f ) * m_sizeScalar ), m_position + ( Vector3f( 0.f, 1.f, -1.f ) * m_sizeScalar ), m_color, m_color, m_lineThickness );
}


//--------------------------------------------------------------------------------------------------------------
void DebugRenderCommandLine::Render()
{
	g_theRenderer->DrawLine( m_startPos, m_endPos, m_color, m_color, m_lineThickness );
}


//--------------------------------------------------------------------------------------------------------------
void DebugRenderCommandArrow::Render()
{
	g_theRenderer->DrawLine( m_startPos, m_endPos, m_color, m_color, m_lineThickness );

	float arrowScalar = .5f;
	Vector3f arrowOffsets = ( m_endPos - m_startPos ) * arrowScalar;
	Vector3f arrowX = Vector3f( m_startPos.x + arrowOffsets.x, m_endPos.y, m_endPos.z );
	Vector3f arrowY = Vector3f( m_endPos.x, m_startPos.y + arrowOffsets.y, m_endPos.z );
	Vector3f arrowZ = Vector3f( m_endPos.x, m_endPos.y, m_startPos.z + arrowOffsets.z );

	g_theRenderer->DrawLine( m_endPos, arrowX, m_color, m_color, m_lineThickness );
	g_theRenderer->DrawLine( m_endPos, arrowY, m_color, m_color, m_lineThickness );
	g_theRenderer->DrawLine( m_endPos, arrowZ, m_color, m_color, m_lineThickness );
}


//--------------------------------------------------------------------------------------------------------------
void DebugRenderCommandAABB3::Render()
{
	
	if ( m_drawFilled ) g_theRenderer->DrawAABB( VertexGroupingRule::AS_TRIANGLES, m_bounds, m_color, m_lineThickness );
	g_theRenderer->DrawAABB( VertexGroupingRule::AS_LINE_STRIP, m_bounds, m_edgeColor, m_lineThickness * 1.5f ); //Ensure lines show over faces.
}


//--------------------------------------------------------------------------------------------------------------
void DebugRenderCommandSphere::Render()
{
	g_theRenderer->DrawSphereGimbal( VertexGroupingRule::AS_LINES, m_centerPos, m_radius, 10.f, m_color, m_lineThickness );
}


//--------------------------------------------------------------------------------------------------------------
void DebugRenderCommandBasis::Render()
{
	g_theRenderer->DrawDebugAxes( m_axisLength, m_lineThickness, m_showZ );
}


//--------------------------------------------------------------------------------------------------------------
void RenderThenExpireDebugCommands3D() //Handles the depth modes.
{
	auto commandIterEnd = g_theDebugRenderCommands->end();
	for ( auto commandIter = g_theDebugRenderCommands->begin(); commandIter != commandIterEnd; )
	{
		DebugRenderCommand* currentCommand = *commandIter;

		switch ( currentCommand->m_depthMode )
		{
		case DEPTH_TEST_ON: 
			g_theRenderer->EnableDepthTesting( true );
			currentCommand->Render();
			break;
		case DEPTH_TEST_OFF:
			g_theRenderer->EnableDepthTesting( false );
			currentCommand->Render();
			break;
		case DEPTH_TEST_DUAL:
			unsigned char alphaBackup = currentCommand->m_color.alphaOpacity;
			float sizeBackup = currentCommand->m_lineThickness;
			g_theRenderer->EnableDepthTesting( false );
			currentCommand->m_color.alphaOpacity >>= 2; //Halved.
			currentCommand->m_lineThickness *= .3f;
			currentCommand->Render();

			g_theRenderer->EnableDepthTesting( true );
			currentCommand->m_color.alphaOpacity = alphaBackup;
			currentCommand->m_lineThickness = sizeBackup;
			currentCommand->Render();
			break;
		}

		if ( currentCommand->IsExpired() ) //Expire after draw or 1-frame commands wouldn't show.
		{
			commandIter = g_theDebugRenderCommands->erase( commandIter );

			delete currentCommand;
			currentCommand = nullptr;
		}
		else ++commandIter;
	}
}


//--------------------------------------------------------------------------------------------------------------
void UpdateDebugCommands( float deltaSeconds )
{
	auto commandIterEnd = g_theDebugRenderCommands->end();
	for ( auto commandIter = g_theDebugRenderCommands->begin(); commandIter != commandIterEnd; ++commandIter )
	{
		DebugRenderCommand* currentCommand = *commandIter;
		currentCommand->Update( deltaSeconds );
	}
}


//--------------------------------------------------------------------------------------------------------------
void ClearDebugCommands() //Else program could shutdown before all commands expire.
{
	auto commandIterEnd = g_theDebugRenderCommands->end();
	for ( auto commandIter = g_theDebugRenderCommands->begin(); commandIter != commandIterEnd; )
	{
		DebugRenderCommand* currentCommand = *commandIter;

		if ( currentCommand != nullptr ) //Expire after draw or 1-frame commands wouldn't show.
		{
			commandIter = g_theDebugRenderCommands->erase( commandIter );

			delete currentCommand;
			currentCommand = nullptr;
		}
		else ++commandIter;
	}
}


//--------------------------------------------------------------------------------------------------------------
void AddDebugRenderCommand( DebugRenderCommand* newCommand )
{
	g_theDebugRenderCommands->push_back( newCommand );
}


//--------------------------------------------------------------------------------------------------------------
void DebugRenderClearCommands( Command& /*args*/ )
{
	ClearDebugCommands();
}
