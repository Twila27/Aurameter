#pragma once

#include "Game/GameCommon.hpp"

//-----------------------------------------------------------------------------
class TheApp;
class Command;

struct HWND__;
struct HDC__;
struct HGLRC__;
struct HINSTANCE__;

typedef HWND__* HWND;
typedef HDC__* HDC;
typedef HGLRC__* HGLRC;
typedef HINSTANCE__* HINSTANCE;


//-----------------------------------------------------------------------------
extern TheApp* g_theApp;


//-----------------------------------------------------------------------------
class TheApp
{

public:

	TheApp();
	void Startup( HINSTANCE applicationInstanceHandle );
	void Shutdown();

	HWND GetWindowHandle();
	HDC GetDisplayDeviceContext();
	HGLRC GetOpenGLRenderingContext();

	double GetScreenWidth() const { return m_screenWidth; }
	double GetScreenHeight() const { return m_screenHeight; }

	static void QuitProgram( Command& );
	static void QuitProgram() { s_isQuitting = true; }
	static bool IsQuitting() { return s_isQuitting; }

	void HandleInput();
	void FlipAndPresent();

private:

	void CreateOpenGLWindow( HINSTANCE applicationInstanceHandle );
	void RunMessagePump();

	double m_screenWidth;
	double m_screenHeight;

	static bool s_isQuitting;

	HWND m_windowHandle;
	HDC m_displayDeviceContext;
	HGLRC m_glRenderContext;
};
