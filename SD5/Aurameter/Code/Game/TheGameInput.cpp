#include "Game/TheGame.hpp"
#include "Engine/Math/Camera2D.hpp"
#include "Engine/Math/Camera3D.hpp"
#include "Engine/Input/TheInput.hpp"
#include "Engine/Math/MathUtils.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC CameraMode TheGame::s_activeCameraMode = CAMERA_MODE_2D;
STATIC Camera3D* TheGame::s_playerCamera3D = new Camera3D( CAMERA3D_DEFAULT_POSITION );
STATIC Camera2D* TheGame::s_playerCamera2D = new Camera2D( CAMERA2D_DEFAULT_POSITION );
STATIC void TheGame::ToggleActiveCamType2D( Command& ) { s_playerCamera2D->m_usesPolarTranslations = !s_playerCamera2D->m_usesPolarTranslations; }
STATIC void TheGame::ToggleActiveCamType3D( Command& ) { s_playerCamera3D->m_usesPolarTranslations = !s_playerCamera3D->m_usesPolarTranslations; }
STATIC void TheGame::ToggleActiveCameraMode( Command& ) { s_activeCameraMode = ( s_activeCameraMode == CAMERA_MODE_2D ) ? CAMERA_MODE_3D : CAMERA_MODE_2D; }


//-----------------------------------------------------------------------------
const Camera3D* TheGame::GetActiveCamera3D() const
{
	return s_playerCamera3D;
}


//-----------------------------------------------------------------------------
const Camera2D* TheGame::GetActiveCamera2D() const
{
	return s_playerCamera2D;
}


//-----------------------------------------------------------------------------
void TheGame::UpdateFromKeyboard2D( float deltaSeconds )
{
	if ( s_playerCamera2D->m_usesPolarTranslations )
		UpdateCameraPolar2D( deltaSeconds );
	else
		UpdateCameraCartesian2D( deltaSeconds );
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::UpdateCameraCartesian2D( float deltaSeconds )
{
	WorldCoords2D& camPos = s_playerCamera2D->m_worldPosition;
	float& camSpeed = s_playerCamera2D->m_speedScale;
	WorldCoords2D camForwardXY = s_playerCamera2D->GetForwardXY();
	WorldCoords2D camLeftXY = s_playerCamera2D->GetLeftXY();

	float deltaMove = camSpeed * deltaSeconds;

	if ( g_theInput->IsKeyDown( KEY_TO_MOVE_UP_2D ) )
		camPos += camLeftXY * deltaMove;

	if ( g_theInput->IsKeyDown( KEY_TO_MOVE_DOWN_2D ) )
		camPos -= camLeftXY * deltaMove;

	if ( g_theInput->IsKeyDown( KEY_TO_MOVE_RIGHT_2D ) )
		camPos += camForwardXY * deltaMove;

	if ( g_theInput->IsKeyDown( KEY_TO_MOVE_LEFT_2D ) )
		camPos -= camForwardXY * deltaMove;

	if ( g_theInput->IsKeyDown( KEY_TO_SLOWDOWN_CAMERA ) )
		camSpeed += deltaMove;

	if ( g_theInput->IsKeyDown( KEY_TO_SPEEDUP_CAMERA ) )
		camSpeed -= deltaMove;
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::UpdateCameraPolar2D( float deltaSeconds )
{
	WorldCoords2D& camPos = s_playerCamera2D->m_worldPosition;
	float& camSpeed = s_playerCamera2D->m_speedScale;
	float radius = camPos.CalcFloatLength();
	float theta = atan2f( camPos.y, camPos.x );

	float deltaMove = camSpeed * deltaSeconds;

	if ( g_theInput->IsKeyDown( KEY_TO_MOVE_LEFT_2D ) )
		theta -= deltaMove;

	if ( g_theInput->IsKeyDown( KEY_TO_MOVE_RIGHT_2D ) )
		theta += deltaMove;

	if ( g_theInput->IsKeyDown( KEY_TO_MOVE_UP_2D ) )
		radius += deltaMove;

	if ( g_theInput->IsKeyDown( KEY_TO_MOVE_DOWN_2D ) )
		radius -= deltaMove;

	if ( g_theInput->IsKeyDown( KEY_TO_SLOWDOWN_CAMERA ) )
		camSpeed += deltaMove;

	if ( g_theInput->IsKeyDown( KEY_TO_SPEEDUP_CAMERA ) )
		camSpeed -= deltaMove;

	camPos = WorldCoords2D( CosDegrees( theta ), SinDegrees( theta ) ) * radius;

	return;
}


//-----------------------------------------------------------------------------
void TheGame::UpdateFromMouse2D()
{
	const float mouseSensitivityYaw = 0.044f;
	s_playerCamera2D->m_orientationDegrees -= mouseSensitivityYaw * (float)g_theInput->GetCursorDeltaX();
}


//-----------------------------------------------------------------------------
void TheGame::UpdateCamera( float deltaSeconds )
{
	UNREFERENCED( deltaSeconds );

	switch ( s_activeCameraMode )
	{
	case CAMERA_MODE_2D:
//		UpdateFromKeyboard2D( deltaSeconds );
//		UpdateFromMouse2D();
		s_playerCamera2D->SatisfyConstraints();
		break;
	case CAMERA_MODE_3D:
		ERROR_RECOVERABLE( "This game does not support Camera3D." );
		s_activeCameraMode = CAMERA_MODE_2D;
		break;
	}
}