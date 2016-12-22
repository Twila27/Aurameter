#pragma once


#include "Engine/Math/Vector2.hpp"
#include "Engine/EngineCommon.hpp"


//-----------------------------------------------------------------------------
class TheInput;
class XboxController;


//-----------------------------------------------------------------------------
extern TheInput* g_theInput;
enum Controllers { CONTROLLER_ONE, CONTROLLER_TWO, CONTROLLER_THREE, CONTROLLER_FOUR, NUM_CONTROLLERS };
enum ControllerButtons { A_BUTTON, B_BUTTON, X_BUTTON, Y_BUTTON, LEFT_SHOULDER_BUTTON, RIGHT_SHOULDER_BUTTON, 
	BACK_BUTTON, START_BUTTON, LEFT_STICK_BUTTON, RIGHT_STICK_BUTTON, NUM_BUTTONS };


//-----------------------------------------------------------------------------
struct KeyButtonState
{
	bool m_isKeyDown;
	bool m_didKeyJustChange;
};


//-----------------------------------------------------------------------------
struct MouseButtonState
{
	bool m_isMouseButtonDown;
	bool m_didMouseButtonJustChange;
};


//-----------------------------------------------------------------------------
class TheInput
{
public:
	TheInput();
	~TheInput();
	void Update();


	//Keyboard.
	void SetKeyDownStatus( unsigned char asKey, bool isNowDown ); //Used by WinMain key callback.
	void SetMouseDownStatus( int windowsButtonEvent, bool isNowDown );
	TODO( "Create a variant of below and similar methods to take an array of inputs." );
	bool IsKeyDown( unsigned char keyID ) const;
	bool WasKeyJustPressed( unsigned char keyID ) const;
	bool WasKeyPressedOnce( unsigned char keyID ) const;


	//Controller.
	bool HasController( Controllers controllerID = NUM_CONTROLLERS ) const;
	bool IsButtonDown( ControllerButtons buttonID, Controllers controllerID ) const;
	bool WasButtonJustPressed( ControllerButtons buttonID, Controllers controllerID ) const;
	bool WasButtonPressedOnce( ControllerButtons buttonID, Controllers controllerID ) const;

	Vector2f GetLeftStickPosition( Controllers controllerID ) const;
	float GetLeftStickPositionAsRadius( Controllers controllerID ) const;
	float GetLeftStickPositionAsAngleInRadians( Controllers controllerID ) const;

	Vector2f GetRightStickPosition( Controllers controllerID ) const;
	float GetRightStickPositionAsRadius( Controllers controllerID ) const;
	float GetRightStickPositionAsAngleInRadians( Controllers controllerID ) const;

	float GetLeftTrigger( Controllers controllerID ) const;
	float GetRightTrigger( Controllers controllerID ) const;


	//Mouse.
	bool IsMouseButtonDown( int windowsButtonEvent ) const;
	bool WasMouseButtonJustPressed( int windowsButtonEvent ) const;
	bool WasMouseButtonPressedOnce( int windowsButtonEvent );

	Vector2i GetCursorPosition() const { return m_cursorPos; }
	void SetCursorPosition( const Vector2i& cursorPos );
	void SetCursorSnapToPos( const Vector2i& snapPos ) { m_snapToPos = snapPos; }

	float GetCursorDeltaX() { return (float)m_cursorDelta.x; }
	float GetCursorDeltaY() { return (float)m_cursorDelta.y; }

	void DisplayCursor() { m_isCursorVisible = true;  }
	void HideCursor() { m_isCursorVisible = false; }

	void OnGainedFocus() { m_hasFocus = true; }
	void OnLostFocus() { m_hasFocus = false; }

	int GetMouseWheelDelta() const { return m_mouseWheelDelta; }
	void SetMouseWheelDelta( short wheelDelta );
	int GetMouseButtonIndexFromWindowsMouseButtonEvent( int windowsButtonEvent ) const;
private:
	static const int NUM_KEYS = 250;
	KeyButtonState m_keys[ NUM_KEYS ];

	static const int WM_LEFT_MOUSE_BUTTON;
	static const int WM_RIGHT_MOUSE_BUTTON;
	static const int NUM_MOUSE_BUTTONS = 2; 
	MouseButtonState m_mouseButtons[ NUM_MOUSE_BUTTONS ]; 

	XboxController* m_controllers[ Controllers::NUM_CONTROLLERS ];
	
	Vector2i m_cursorPos;
	Vector2i m_snapToPos;
	Vector2i m_cursorDelta;
	bool m_isCursorVisible;

	bool m_hasFocus;

	static const int MOUSEWHEEL_DELTA_STANDARD_AMOUNT = 120;
	int m_mouseWheelDelta;
};
