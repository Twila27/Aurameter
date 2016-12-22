#pragma once


//--------------------------------------------------------------------------------------------------------------
class TheEngine
{
public:
	void RunFrame();
	void Startup( double screenWidth, double screenHeight );
	void Shutdown();
	bool IsQuitting();

private:
	void Render();
	void Update( float deltaSeconds );

	void RenderDebug3D();
	void RenderDebug2D();

	void RenderLeftDebugText2D();
	void RenderRightDebugText2D();
	void RenderDebugMemoryWindow();
};


//--------------------------------------------------------------------------------------------------------------
extern TheEngine* g_theEngine;
