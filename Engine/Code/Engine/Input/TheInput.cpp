#include "Engine/Input/TheInput.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"


#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#include "Engine/Input/XboxController.hpp"


//--------------------------------------------------------------------------------------------------------------
TheInput* g_theInput = nullptr;

const int TheInput::WM_LEFT_MOUSE_BUTTON = 0x01;
const int TheInput::WM_RIGHT_MOUSE_BUTTON = 0x02;


//--------------------------------------------------------------------------------------------------------------
TheInput::TheInput()
	: m_mouseWheelDelta( 0 )
{
	for ( int keyIndex = 0; keyIndex < NUM_KEYS; ++keyIndex )
	{
		m_keys[ keyIndex ].m_isKeyDown = false;
		m_keys[ keyIndex ].m_didKeyJustChange = false;
	}
	for ( int mouseButtonIndex = 0; mouseButtonIndex < NUM_MOUSE_BUTTONS; ++mouseButtonIndex )
	{
		m_mouseButtons[ mouseButtonIndex ].m_isMouseButtonDown = false;
		m_mouseButtons[ mouseButtonIndex ].m_didMouseButtonJustChange = false;
	}
	for ( int controllerSlot = 0; controllerSlot < NUM_CONTROLLERS; ++controllerSlot )
	{
		m_controllers[ controllerSlot ] = nullptr;
	}
}


//--------------------------------------------------------------------------------------------------------------
TheInput::~TheInput()
{
	for ( int controllerIndex = 0; controllerIndex < Controllers::NUM_CONTROLLERS; controllerIndex++ )
	{
		if ( m_controllers[ controllerIndex ] != nullptr )
		{
			delete m_controllers[ controllerIndex ];
			m_controllers[ controllerIndex ] = nullptr;
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
void TheInput::SetKeyDownStatus( unsigned char asKey, bool isNowDown )
{
	if ( asKey >= NUM_KEYS ) return;

	m_keys[ asKey ].m_didKeyJustChange = ( m_keys[ asKey ].m_isKeyDown != isNowDown ? true : false );
	m_keys[ asKey ].m_isKeyDown = isNowDown;
}


//--------------------------------------------------------------------------------------------------------------
void TheInput::SetMouseDownStatus( int windowsMouseButtonEvent, bool isNowDown )
{
	int mouseButtonIndex = GetMouseButtonIndexFromWindowsMouseButtonEvent( windowsMouseButtonEvent );

	m_mouseButtons[ mouseButtonIndex ].m_didMouseButtonJustChange =
		( m_mouseButtons[ mouseButtonIndex ].m_isMouseButtonDown != isNowDown ? true : false );
	m_mouseButtons[ mouseButtonIndex ].m_isMouseButtonDown = isNowDown;
}


//--------------------------------------------------------------------------------------------------------------
bool TheInput::IsKeyDown( unsigned char keyID ) const
{
	return m_keys[ keyID ].m_isKeyDown;
}


//--------------------------------------------------------------------------------------------------------------
bool TheInput::WasKeyJustPressed( unsigned char keyID ) const
{
	return m_keys[ keyID ].m_didKeyJustChange;
}


//--------------------------------------------------------------------------------------------------------------
bool TheInput::WasKeyPressedOnce( unsigned char keyID ) const
{
	return m_keys[ keyID ].m_didKeyJustChange && m_keys[ keyID ].m_isKeyDown;
}


//--------------------------------------------------------------------------------------------------------------
bool TheInput::HasController( Controllers controllerID /*= NUM_CONTROLLERS */ ) const
{
	if ( controllerID != NUM_CONTROLLERS )
		return ( m_controllers[ controllerID ] != nullptr );

	for ( int index = 0; index < NUM_CONTROLLERS; index++ )
		if ( m_controllers[ index ] != nullptr )
			return true;

	return false;
}


//--------------------------------------------------------------------------------------------------------------
bool TheInput::IsMouseButtonDown( int windowsMouseButtonEvent ) const
{
	int mouseButtonIndex = GetMouseButtonIndexFromWindowsMouseButtonEvent( windowsMouseButtonEvent );

	return m_mouseButtons[ mouseButtonIndex ].m_isMouseButtonDown;
}


//--------------------------------------------------------------------------------------------------------------
bool TheInput::WasMouseButtonJustPressed( int windowsMouseButtonEvent ) const
{
	int mouseButtonIndex = GetMouseButtonIndexFromWindowsMouseButtonEvent( windowsMouseButtonEvent );

	return m_mouseButtons[ mouseButtonIndex ].m_didMouseButtonJustChange;
}


//--------------------------------------------------------------------------------------------------------------
bool TheInput::WasMouseButtonPressedOnce( int windowsMouseButtonEvent )
{
	int mouseButtonIndex = GetMouseButtonIndexFromWindowsMouseButtonEvent( windowsMouseButtonEvent );

	bool retval = m_mouseButtons[ mouseButtonIndex ].m_didMouseButtonJustChange
		&& m_mouseButtons[ mouseButtonIndex ].m_isMouseButtonDown;

	//First come first serve, because WM_LBUTTONDOWN is proving slower than my block breaks!

	m_mouseButtons[ mouseButtonIndex ].m_didMouseButtonJustChange = false;

	return retval;
}


//--------------------------------------------------------------------------------------------------------------
bool TheInput::IsButtonDown( ControllerButtons buttonID, Controllers controllerID ) const
{
	if ( m_controllers[ controllerID ] == nullptr )
		return false;

	switch ( buttonID )
	{
	case ControllerButtons::A_BUTTON: return m_controllers[ controllerID ]->IsAButtonDown();
	case ControllerButtons::B_BUTTON: return m_controllers[ controllerID ]->IsBButtonDown();
	case ControllerButtons::X_BUTTON: return m_controllers[ controllerID ]->IsXButtonDown();
	case ControllerButtons::Y_BUTTON: return m_controllers[ controllerID ]->IsYButtonDown();
	case ControllerButtons::LEFT_SHOULDER_BUTTON: return m_controllers[ controllerID ]->IsLeftShoulderButtonDown();
	case ControllerButtons::RIGHT_SHOULDER_BUTTON: return m_controllers[ controllerID ]->IsRightShoulderButtonDown();
	case ControllerButtons::BACK_BUTTON: return m_controllers[ controllerID ]->IsBackButtonDown();
	case ControllerButtons::START_BUTTON: return m_controllers[ controllerID ]->IsStartButtonDown();
	case ControllerButtons::LEFT_STICK_BUTTON: return m_controllers[ controllerID ]->IsLeftStickClickedIn();
	case ControllerButtons::RIGHT_STICK_BUTTON: return m_controllers[ controllerID ]->IsRightStickClickedIn();
	default: ERROR_AND_DIE( "Invalid ButtonID in TheInput::IsButtonDown()!" );
	}
}


//--------------------------------------------------------------------------------------------------------------
bool TheInput::WasButtonJustPressed( ControllerButtons buttonID, Controllers controllerID ) const
{
	if ( m_controllers[ controllerID ] == nullptr )
		return false;

	switch ( buttonID )
	{
		case ControllerButtons::A_BUTTON: return m_controllers[ controllerID ]->WasAButtonJustPressed();
		case ControllerButtons::B_BUTTON: return m_controllers[ controllerID ]->WasBButtonJustPressed();
		case ControllerButtons::X_BUTTON: return m_controllers[ controllerID ]->WasXButtonJustPressed();
		case ControllerButtons::Y_BUTTON: return m_controllers[ controllerID ]->WasYButtonJustPressed();
		case ControllerButtons::LEFT_SHOULDER_BUTTON: return m_controllers[ controllerID ]->WasLeftShoulderButtonJustPressed();
		case ControllerButtons::RIGHT_SHOULDER_BUTTON: return m_controllers[ controllerID ]->WasRightShoulderButtonJustPressed();
		case ControllerButtons::BACK_BUTTON: return m_controllers[ controllerID ]->WasBackButtonJustPressed();
		case ControllerButtons::START_BUTTON: return m_controllers[ controllerID ]->WasStartButtonJustPressed();
		case ControllerButtons::LEFT_STICK_BUTTON: return m_controllers[ controllerID ]->WasLeftStickClickJustPressed();
		case ControllerButtons::RIGHT_STICK_BUTTON: return m_controllers[ controllerID ]->WasRightStickClickJustPressed();
		default: ERROR_AND_DIE( "Invalid ButtonID in TheInput::WasButtonJustPressed()!" );
	}
}


//--------------------------------------------------------------------------------------------------------------
bool TheInput::WasButtonPressedOnce( ControllerButtons buttonID, Controllers controllerID ) const
{
	if ( m_controllers[ controllerID ] == nullptr )
		return false;

	return IsButtonDown( buttonID, controllerID ) && WasButtonPressedOnce( buttonID, controllerID );
}


//--------------------------------------------------------------------------------------------------------------
Vector2f TheInput::GetLeftStickPosition( Controllers controllerID ) const
{
	return m_controllers[ controllerID ]->GetLeftStickPosition();
}


//--------------------------------------------------------------------------------------------------------------
float TheInput::GetLeftStickPositionAsRadius( Controllers controllerID ) const
{
	return m_controllers[ controllerID ]->GetLeftStickPositionAsRadius();
}


//--------------------------------------------------------------------------------------------------------------
float TheInput::GetLeftStickPositionAsAngleInRadians( Controllers controllerID ) const
{
	return m_controllers[ controllerID ]->GetLeftStickPositionAsAngleInRadians();
}


//--------------------------------------------------------------------------------------------------------------
Vector2f TheInput::GetRightStickPosition( Controllers controllerID ) const
{
	return m_controllers[ controllerID ]->GetRightStickPosition();
}


//--------------------------------------------------------------------------------------------------------------
float TheInput::GetRightStickPositionAsRadius( Controllers controllerID ) const
{
	return m_controllers[ controllerID ]->GetRightStickPositionAsRadius();
}


//--------------------------------------------------------------------------------------------------------------
float TheInput::GetRightStickPositionAsAngleInRadians( Controllers controllerID ) const
{
	return m_controllers[ controllerID ]->GetRightStickPositionAsAngleInRadians();
}


//--------------------------------------------------------------------------------------------------------------
float TheInput::GetLeftTrigger( Controllers controllerID ) const
{
	return m_controllers[ controllerID ]->GetLeftTrigger( );
}


//--------------------------------------------------------------------------------------------------------------
float TheInput::GetRightTrigger( Controllers controllerID ) const
{
	return m_controllers[ controllerID ]->GetRightTrigger();
}


//--------------------------------------------------------------------------------------------------------------
void TheInput::SetCursorPosition( const Vector2i& cursorPos )
{
	SetCursorPos( cursorPos.x, cursorPos.y );
}


//--------------------------------------------------------------------------------------------------------------
void TheInput::SetMouseWheelDelta( short wheelDelta )
{
	//+120 is one increment forward, -120 is one back.
	m_mouseWheelDelta = wheelDelta / MOUSEWHEEL_DELTA_STANDARD_AMOUNT;
}


//--------------------------------------------------------------------------------------------------------------
int TheInput::GetMouseButtonIndexFromWindowsMouseButtonEvent( int windowsMouseButtonEvent ) const
{
	switch ( windowsMouseButtonEvent )
	{
		case WM_LEFT_MOUSE_BUTTON: return 0;
		case WM_RIGHT_MOUSE_BUTTON: return 1;
		default: ERROR_RECOVERABLE( "windowsMouseButtonEvent Outside Mouse Array" ); return -1;
	}
}


//--------------------------------------------------------------------------------------------------------------
void TheInput::Update()
{
	//Keyboard.
	for ( int keyIndex = 0; keyIndex < NUM_KEYS; ++keyIndex )
		m_keys[ keyIndex ].m_didKeyJustChange = false;

	//Controller updates, drop-ins/outs.
	XINPUT_STATE xboxControllerState;
	memset( &xboxControllerState, 0, sizeof( xboxControllerState ) );
	for ( int controllerSlot = 0; controllerSlot < 3; controllerSlot++ )
	{
		DWORD errorStatus = XInputGetState( controllerSlot, &xboxControllerState );
		if ( errorStatus == ERROR_SUCCESS )
		{
			if ( m_controllers[ controllerSlot ] == nullptr )
			{
				m_controllers[ controllerSlot ] = new XboxController( controllerSlot );
			}
			m_controllers[ controllerSlot ]->Update();
		}
		else if ( errorStatus == ERROR_DEVICE_NOT_CONNECTED )
		{
			if ( m_controllers[ controllerSlot ] != nullptr )
			{
				delete m_controllers[ controllerSlot ];
				m_controllers[ controllerSlot ] = nullptr;
			}
		}
	}

	//Mouse.
	if ( m_hasFocus )
	{
		POINT cursorPos; //Windows-only class.
		GetCursorPos( &cursorPos );
		m_cursorPos = Vector2i( cursorPos.x, cursorPos.y );

		m_cursorDelta = ( m_cursorPos - m_snapToPos );

		SetCursorPos( m_snapToPos.x, m_snapToPos.y );

		ShowCursor( m_isCursorVisible );
	}
}
