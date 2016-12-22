#include "Game/TheApp.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>

#include "Engine/TheEngine.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Core/Command.hpp"
#include "Engine/Core/TheConsole.hpp"
#include "Engine/Input/TheInput.hpp"


//--------------------------------------------------------------------------------------------------------------
TheApp* g_theApp = nullptr;
STATIC bool TheApp::s_isQuitting = false;


//--------------------------------------------------------------------------------------------------------------
HWND TheApp::GetWindowHandle() { return m_windowHandle; }
HDC TheApp::GetDisplayDeviceContext() { return m_displayDeviceContext; }
HGLRC TheApp::GetOpenGLRenderingContext() { return m_glRenderContext; }


//--------------------------------------------------------------------------------------------------------------
const int OFFSET_FROM_WINDOWS_DESKTOP = 50;
const int WINDOW_PHYSICAL_WIDTH = 1600;
const int WINDOW_PHYSICAL_HEIGHT = 900;
const float VIEW_LEFT = 0.0;
const float VIEW_RIGHT = 1600.0;
const float VIEW_BOTTOM = 0.0;
const float VIEW_TOP = VIEW_RIGHT * static_cast<float>( WINDOW_PHYSICAL_HEIGHT ) / static_cast<float>( WINDOW_PHYSICAL_WIDTH );


//--------------------------------------------------------------------------------------------------------------
void TheApp::QuitProgram( Command& /*args*/ )
{
	s_isQuitting = true;
}


//--------------------------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowsMessageHandlingProcedure( HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam )
{
	unsigned char asKey = (unsigned char)wParam;

	if ( g_theConsole != nullptr && g_theConsole->IsVisible() && g_theConsole->ShowingPrompt() )
	{
		if ( wmMessageCode == WM_CHAR )
			g_theConsole->UpdatePromptForChar( asKey );
		if ( wmMessageCode == WM_KEYDOWN )
			g_theConsole->UpdatePromptForKeydown( asKey );
	}
	else
	{
		switch ( wmMessageCode )
		{
		case WM_CLOSE:
		case WM_DESTROY:
		case WM_QUIT:
			return 0;

		case WM_KEYDOWN:
			g_theInput->SetKeyDownStatus( asKey, true );
			if ( asKey == VK_ESCAPE )
			{
				g_theApp->QuitProgram();
				return 0;
			}
			break;

		case WM_KEYUP:
			g_theInput->SetKeyDownStatus( asKey, false );
			break;

		case WM_LBUTTONDOWN:
			g_theInput->SetMouseDownStatus( VK_LBUTTON, true );
			break;


		case WM_LBUTTONUP:
			g_theInput->SetMouseDownStatus( VK_LBUTTON, false );
			break;


		case WM_RBUTTONDOWN:
			g_theInput->SetMouseDownStatus( VK_RBUTTON, true );
			break;


		case WM_RBUTTONUP:
			g_theInput->SetMouseDownStatus( VK_RBUTTON, false );
			break;
		}
	}

	switch ( wmMessageCode ) //Some events we still want to run even if console's up.
	{
	case WM_SETFOCUS:
		if ( g_theInput != nullptr )
			g_theInput->OnGainedFocus();
		break;

	case WM_KILLFOCUS:
		if ( g_theInput != nullptr )
			g_theInput->OnLostFocus();
		break;

	case WM_MOUSEWHEEL:
		g_theInput->SetMouseWheelDelta( GET_WHEEL_DELTA_WPARAM( wParam ) );
		break;
	}
	return DefWindowProc( windowHandle, wmMessageCode, wParam, lParam );
}


//--------------------------------------------------------------------------------------------------------------
void TheApp::CreateOpenGLWindow( HINSTANCE applicationInstanceHandle )
{
	// Define a window class
	WNDCLASSEX windowClassDescription;
	memset( &windowClassDescription, 0, sizeof( windowClassDescription ) );
	windowClassDescription.cbSize = sizeof( windowClassDescription );
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>( WindowsMessageHandlingProcedure ); // Assign a win32 message-handling function
	windowClassDescription.hInstance = GetModuleHandle( NULL );
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT( "Simple Window Class" );
	RegisterClassEx( &windowClassDescription );

	const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect( desktopWindowHandle, &desktopRect );

	RECT windowRect = { OFFSET_FROM_WINDOWS_DESKTOP, OFFSET_FROM_WINDOWS_DESKTOP, OFFSET_FROM_WINDOWS_DESKTOP + WINDOW_PHYSICAL_WIDTH, OFFSET_FROM_WINDOWS_DESKTOP + WINDOW_PHYSICAL_HEIGHT };
	AdjustWindowRectEx( &windowRect, windowStyleFlags, FALSE, windowStyleExFlags );

	WCHAR windowTitle[ 1024 ];
	MultiByteToWideChar( GetACP(), 0, g_appName, -1, windowTitle, sizeof( windowTitle ) / sizeof( windowTitle[ 0 ] ) );
	m_windowHandle = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		applicationInstanceHandle,
		NULL );

	ShowWindow( m_windowHandle, SW_SHOW );
	SetForegroundWindow( m_windowHandle );
	SetFocus( m_windowHandle );

	m_displayDeviceContext = GetDC( m_windowHandle );

	HCURSOR cursor = LoadCursor( NULL, IDC_ARROW );
	SetCursor( cursor );

	PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
	memset( &pixelFormatDescriptor, 0, sizeof( pixelFormatDescriptor ) );
	pixelFormatDescriptor.nSize = sizeof( pixelFormatDescriptor );
	pixelFormatDescriptor.nVersion = 1;
	pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
	pixelFormatDescriptor.cColorBits = 24;
	pixelFormatDescriptor.cDepthBits = 24;
	pixelFormatDescriptor.cAccumBits = 0;
	pixelFormatDescriptor.cStencilBits = 8;

	int pixelFormatCode = ChoosePixelFormat( m_displayDeviceContext, &pixelFormatDescriptor );
	SetPixelFormat( m_displayDeviceContext, pixelFormatCode, &pixelFormatDescriptor );
	m_glRenderContext = wglCreateContext( m_displayDeviceContext );
	wglMakeCurrent( m_displayDeviceContext, m_glRenderContext );
}


//--------------------------------------------------------------------------------------------------------------
void TheApp::RunMessagePump()
{
	MSG queuedMessage;
	for ( ;; )
	{
		const BOOL wasMessagePresent = PeekMessage( &queuedMessage, NULL, 0, 0, PM_REMOVE );
		if ( !wasMessagePresent )
		{
			break;
		}

		TranslateMessage( &queuedMessage );
		DispatchMessage( &queuedMessage );
	}
}


//--------------------------------------------------------------------------------------------------------------
void TheApp::HandleInput()
{
	g_theInput->Update(); //Must come first, else WasJustPressed values will be invalidated.

	RunMessagePump(); //Handles firing the callback to WindowsMessageHandlingProcedure set by CreateWindow.
}


//--------------------------------------------------------------------------------------------------------------
void TheApp::FlipAndPresent()
{
	SwapBuffers( m_displayDeviceContext );
}


//--------------------------------------------------------------------------------------------------------------
TheApp::TheApp()
	: m_screenWidth( VIEW_RIGHT )
	, m_screenHeight( VIEW_TOP )
{
}


//--------------------------------------------------------------------------------------------------------------
void TheApp::Startup( HINSTANCE applicationInstanceHandle )
{
	SetProcessDPIAware();
	CreateOpenGLWindow( applicationInstanceHandle );

	g_theEngine = new TheEngine();
	g_theEngine->Startup( m_screenWidth, m_screenHeight ); //Subsystem Allocations Occur Here
	g_theConsole->RegisterCommand( "Quit", QuitProgram );
}


//--------------------------------------------------------------------------------------------------------------
void TheApp::Shutdown()
{
	g_theEngine->Shutdown();
	delete g_theEngine;
	g_theEngine = nullptr;
}