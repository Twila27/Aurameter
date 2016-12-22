#include <Windows.h>
#include <Xinput.h> //Include XInput API.
#pragma comment( lib, "xinput9_1_0" ) //Link in xinput.lib static library.  // #Eiserloh: Explicitly use XInput v9_1_0, since v1_4 is not supported under Windows 7
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/XboxController.hpp"

#include <math.h>
#include <float.h>

const int A_BITMASK = 0x00001000;
const int B_BITMASK = 0x00002000;
const int X_BITMASK = 0x00004000;
const int Y_BITMASK = 0x00008000;
const int LEFT_SHOULDER_BITMASK = 0x00000100;
const int RIGHT_SHOULDER_BITMASK = 0x00000200;
const int START_BITMASK = 0x00000010;
const int BACK_BITMASK = 0x00000020;
const int LEFT_STICK_CLICK_BITMASK = 0x00000040;
const int RIGHT_STICK_CLICK_BITMASK = 0x00000080;
const float XINPUT_GAMEPAD_LEFT_STICK_INNER_DEADZONE = 7849.f;
const float XINPUT_GAMEPAD_RIGHT_STICK_INNER_DEADZONE = 8689.f;
const float XINPUT_GAMEPAD_TRIGGER_DEADZONE = 30.f;
float const STICK_OUTER_DEADZONE_THRESHOLD = 32000.f;
const float TRIGGER_VALUE_MAX = 255.f;


//---------------------------------------------------------------------------
XboxController::XboxController( )
	: m_controllerNumber( 0 )
{
}


//---------------------------------------------------------------------------
XboxController::XboxController( int controllerNumber )
	: m_controllerNumber( controllerNumber )
{
}

//---------------------------------------------------------------------------
void XboxController::MapControlSticks( short sThumbLX, short sThumbLY, short sThumbRX, short sThumbRY )
{
	float leftStickThetaRadians = atan2( static_cast<float>( sThumbLY ), static_cast<float>( sThumbLX ) ); //In radians, hold right for 0, up +pi/2, down -pi/2, left +-pi.
	float rightStickThetaRadians = atan2( static_cast<float>( sThumbRY ), static_cast<float>( sThumbRX ) );

	float leftStickRadius = sqrt( ( (float)sThumbLX * (float)sThumbLX ) + ( (float)sThumbLY * (float)sThumbLY ) );
	float rightStickRadius = sqrt( ( (float)sThumbRX * (float)sThumbRX ) + ( (float)sThumbRY * (float)sThumbRY ) );

	float correctedLeftStickRadius = -1.f;
	float correctedRightStickRadius = -1.f;

	if ( leftStickRadius < XINPUT_GAMEPAD_LEFT_STICK_INNER_DEADZONE )
	{
		correctedLeftStickRadius = 0.f;
	}
	else if ( leftStickRadius > STICK_OUTER_DEADZONE_THRESHOLD )
	{
		correctedLeftStickRadius = 1.f;
	}
	else
	{
		correctedLeftStickRadius = RangeMap( leftStickRadius, XINPUT_GAMEPAD_LEFT_STICK_INNER_DEADZONE, STICK_OUTER_DEADZONE_THRESHOLD, 0.f, 1.f );
	}

	if ( rightStickRadius < XINPUT_GAMEPAD_RIGHT_STICK_INNER_DEADZONE )
	{
		correctedRightStickRadius = 0.f;
	}
	else if ( rightStickRadius > STICK_OUTER_DEADZONE_THRESHOLD )
	{
		correctedRightStickRadius = 1.f;
	}
	else 
	{
		correctedRightStickRadius = RangeMap( rightStickRadius, XINPUT_GAMEPAD_RIGHT_STICK_INNER_DEADZONE, STICK_OUTER_DEADZONE_THRESHOLD, 0.f, 1.f );
	}
	m_correctedLeftStick.x = correctedLeftStickRadius * cos( leftStickThetaRadians );
	m_correctedLeftStick.y = correctedLeftStickRadius * sin( leftStickThetaRadians );

	m_correctedRightStick.x = correctedRightStickRadius * cosf( rightStickThetaRadians );
	m_correctedRightStick.y = correctedRightStickRadius * sinf( rightStickThetaRadians );

	m_correctedLeftStickInPolar.radius = correctedLeftStickRadius;
	m_correctedRightStickInPolar.radius = correctedRightStickRadius;

	//Corrects theta's non-monotonic interval to be made monotonic, from 0 to 2pi.
	m_correctedLeftStickInPolar.thetaRadians = leftStickThetaRadians + ( leftStickThetaRadians < 0.f ? fTWO_PI : 0.f );
	m_correctedRightStickInPolar.thetaRadians = rightStickThetaRadians + ( rightStickThetaRadians < 0.f ? fTWO_PI : 0.f );
}


//-----------------------------------------------------------------------------
void XboxController::UpdateButtonStates( WORD wButtons )
{
	bool resultOfBitmasks;

	resultOfBitmasks = ( wButtons & A_BITMASK ) > 0;
	m_aButton.m_didButtonJustChange = ( m_aButton.m_isButtonDown != resultOfBitmasks ? true : false );
	m_aButton.m_isButtonDown = resultOfBitmasks;

	resultOfBitmasks = ( wButtons & B_BITMASK ) > 0;
	m_bButton.m_didButtonJustChange = ( m_bButton.m_isButtonDown != resultOfBitmasks ? true : false );
	m_bButton.m_isButtonDown = resultOfBitmasks;

	resultOfBitmasks = ( wButtons & X_BITMASK ) > 0;
	m_xButton.m_didButtonJustChange = ( m_xButton.m_isButtonDown != resultOfBitmasks ? true : false );
	m_xButton.m_isButtonDown = resultOfBitmasks;

	resultOfBitmasks = ( wButtons & Y_BITMASK ) > 0;
	m_yButton.m_didButtonJustChange = ( m_yButton.m_isButtonDown != resultOfBitmasks ? true : false );
	m_yButton.m_isButtonDown = resultOfBitmasks;

	resultOfBitmasks = ( wButtons & LEFT_SHOULDER_BITMASK) > 0;
	m_leftShoulderButton.m_didButtonJustChange = ( m_leftShoulderButton.m_isButtonDown != resultOfBitmasks ? true : false );
	m_leftShoulderButton.m_isButtonDown = resultOfBitmasks;

	resultOfBitmasks = ( wButtons & RIGHT_SHOULDER_BITMASK ) > 0;
	m_rightShoulderButton.m_didButtonJustChange = ( m_rightShoulderButton.m_isButtonDown != resultOfBitmasks ? true : false );
	m_rightShoulderButton.m_isButtonDown = resultOfBitmasks;

	resultOfBitmasks = ( wButtons & START_BITMASK ) > 0;
	m_startButton.m_didButtonJustChange = ( m_startButton.m_isButtonDown != resultOfBitmasks ? true : false );
	m_startButton.m_isButtonDown = resultOfBitmasks;

	resultOfBitmasks = ( wButtons & BACK_BITMASK ) > 0;
	m_backButton.m_didButtonJustChange = ( m_backButton.m_isButtonDown != resultOfBitmasks ? true : false );
	m_backButton.m_isButtonDown = resultOfBitmasks;

	resultOfBitmasks = ( wButtons & LEFT_STICK_CLICK_BITMASK ) > 0;
	m_leftStickClick.m_didButtonJustChange = ( m_leftStickClick.m_isButtonDown != resultOfBitmasks ? true : false );
	m_leftStickClick.m_isButtonDown = resultOfBitmasks;

	resultOfBitmasks = ( wButtons & RIGHT_STICK_CLICK_BITMASK ) > 0;
	m_rightStickClick.m_didButtonJustChange = ( m_rightStickClick.m_isButtonDown != resultOfBitmasks ? true : false );
	m_rightStickClick.m_isButtonDown = resultOfBitmasks;
}


//-----------------------------------------------------------------------------
void XboxController::Update( )
{
	XINPUT_STATE xboxControllerState;
	memset( &xboxControllerState, 0, sizeof( xboxControllerState ) );
	DWORD errorStatus = XInputGetState( m_controllerNumber, &xboxControllerState );
	if ( errorStatus == ERROR_SUCCESS )
	{
		UpdateButtonStates( xboxControllerState.Gamepad.wButtons );

		float correctedLeftTrigger = xboxControllerState.Gamepad.bLeftTrigger < XINPUT_GAMEPAD_TRIGGER_DEADZONE ? 0.f : static_cast<float>( xboxControllerState.Gamepad.bLeftTrigger );
		float correctedRightTrigger = xboxControllerState.Gamepad.bRightTrigger < XINPUT_GAMEPAD_TRIGGER_DEADZONE ? 0.f : static_cast<float>( xboxControllerState.Gamepad.bRightTrigger );
		m_leftTrigger = correctedLeftTrigger / TRIGGER_VALUE_MAX;
		m_rightTrigger = correctedRightTrigger / TRIGGER_VALUE_MAX;

		MapControlSticks( xboxControllerState.Gamepad.sThumbLX, xboxControllerState.Gamepad.sThumbLY, xboxControllerState.Gamepad.sThumbRX, xboxControllerState.Gamepad.sThumbRY );
		
	}
	else if ( errorStatus == ERROR_DEVICE_NOT_CONNECTED )
	{
		//Print error when allowed to use output: this says that controller slot [controllerNumber] is empty.
	}
	else
	{
		//Print error when allowed to use output.
	}
}

