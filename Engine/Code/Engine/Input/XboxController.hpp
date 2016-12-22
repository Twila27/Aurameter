#pragma once


#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/PolarCoords.hpp"


//-----------------------------------------------------------------------------
struct ButtonState
{
	bool m_isButtonDown = false;
	bool m_didButtonJustChange = false;
};


//-----------------------------------------------------------------------------
class XboxController
{
public:
	XboxController();
	XboxController( int controllerNumber );

	void Update();

	Vector2f GetLeftStickPosition() const { return m_correctedLeftStick; }
	float GetLeftStickPositionAsRadius() const { return m_correctedLeftStickInPolar.radius; }
	float GetLeftStickPositionAsAngleInRadians() const { return m_correctedLeftStickInPolar.thetaRadians; }
		
	Vector2f GetRightStickPosition() const { return m_correctedRightStick; }
	float GetRightStickPositionAsRadius() const { return m_correctedRightStickInPolar.radius; }
	float GetRightStickPositionAsAngleInRadians() const { return m_correctedRightStickInPolar.thetaRadians; }
		
	float GetLeftTrigger() const { return m_leftTrigger; }
	float GetRightTrigger() const { return m_rightTrigger; }
		
	bool IsAButtonDown() const { return m_aButton.m_isButtonDown; }
	bool IsBButtonDown( ) const { return m_bButton.m_isButtonDown; }
	bool IsXButtonDown( ) const { return m_xButton.m_isButtonDown; }
	bool IsYButtonDown( ) const { return m_yButton.m_isButtonDown; }
	bool IsLeftShoulderButtonDown( ) const { return m_leftShoulderButton.m_isButtonDown; }
	bool IsRightShoulderButtonDown( ) const { return m_rightShoulderButton.m_isButtonDown; }
	bool IsBackButtonDown() const { return m_backButton.m_isButtonDown; }
	bool IsStartButtonDown() const { return m_startButton.m_isButtonDown; }
	bool IsLeftStickClickedIn() const { return m_leftStickClick.m_isButtonDown; }
	bool IsRightStickClickedIn() const { return m_rightStickClick.m_isButtonDown; }

	bool WasAButtonJustPressed() const { return m_aButton.m_didButtonJustChange; }
	bool WasBButtonJustPressed() const { return m_bButton.m_didButtonJustChange; }
	bool WasXButtonJustPressed() const { return m_xButton.m_didButtonJustChange; }
	bool WasYButtonJustPressed() const { return m_yButton.m_didButtonJustChange; }
	bool WasLeftShoulderButtonJustPressed() const { return m_leftShoulderButton.m_didButtonJustChange; }
	bool WasRightShoulderButtonJustPressed( ) const { return m_rightShoulderButton.m_didButtonJustChange; }
	bool WasBackButtonJustPressed() const { return m_backButton.m_didButtonJustChange; }
	bool WasStartButtonJustPressed() const { return m_startButton.m_didButtonJustChange; }
	bool WasLeftStickClickJustPressed() const { return m_leftStickClick.m_didButtonJustChange; }
	bool WasRightStickClickJustPressed() const { return m_rightStickClick.m_didButtonJustChange; }

private:
	int m_controllerNumber;

	ButtonState m_aButton;
	ButtonState m_bButton;
	ButtonState m_xButton;
	ButtonState m_yButton;
	ButtonState m_leftShoulderButton;
	ButtonState m_rightShoulderButton;
	ButtonState m_startButton;
	ButtonState m_backButton;
	ButtonState m_leftStickClick;
	ButtonState m_rightStickClick;

	float m_leftTrigger;
	float m_rightTrigger;

	Vector2f m_correctedLeftStick;
	Vector2f m_correctedRightStick;
	PolarCoords<float> m_correctedLeftStickInPolar;
	PolarCoords<float> m_correctedRightStickInPolar;

	void UpdateButtonStates( unsigned short wButtons );
	void MapControlSticks( short sThumbLX, short sThumbLY, short sThumbRX, short sThumbRY );
};