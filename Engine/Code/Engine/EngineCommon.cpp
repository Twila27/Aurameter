#include "Engine/EngineCommon.hpp"


bool g_inDebugMode = false;
bool g_showDebugMemoryWindow = false;
DebugRenderMode g_currentDebugRenderMode = RENDER_MODE_NONE;
float ENGINE_PERSPECTIVE_FOV_Y_DEGREES = 60.f;
float ENGINE_Z_NEAR = .1f;
float ENGINE_Z_FAR = 1000.f;

//--------------------------------------------------------------------------------------------------------------
#define GL_POINTS                         0x0000
#define GL_LINES                          0x0001
#define GL_LINE_LOOP                      0x0002
#define GL_LINE_STRIP                     0x0003
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_STRIP                 0x0005
#define GL_TRIANGLE_FAN                   0x0006
unsigned int GetOpenGLVertexGroupingRule( unsigned int engineVertexGroupingRule )
{
	switch ( engineVertexGroupingRule )
	{
		case VertexGroupingRule::AS_LINES: return GL_LINES;
		case VertexGroupingRule::AS_POINTS: return GL_POINTS;
		case VertexGroupingRule::AS_LINE_LOOP: return GL_LINE_LOOP;
		case VertexGroupingRule::AS_LINE_STRIP: return GL_LINE_STRIP;
		case VertexGroupingRule::AS_TRIANGLES: return GL_TRIANGLES;
		default: return GL_POINTS;
	}
}


//--------------------------------------------------------------------------------------------------------------
Matrix4x4f GetWorldChangeOfBasis( Ordering ordering )
{
	//Having to send in [down, back, left] because of transpose.
	//My basis turns the default [x y z] to [-y z x], like SD2 SimpleMiner, 
	//But projection matrix then inverts k-vector, so we need to pass a matrix to turn [x y z] to [-y z -x].

	Matrix4x4f changeOfBasisTransform( ordering );
	if ( ordering == COLUMN_MAJOR )
	{
		const float values[ 16 ] = {
			0.f,	-1.f,	0.f,	0.f, // i == 0,-1,0 == -y for world right.
			0.f,	0.f,	1.f,	0.f, // j == 0, 0,1 == +z for world up.
			-1.f,	0.f,	0.f,	0.f, // k == -1,0,0 == -x for world BACKWARD, after perspective projection inverts k-vector.
			0.f,	0.f,	0.f,	1.f,
		};
		changeOfBasisTransform.SetAllValuesAssumingSameOrdering( values );
	}
	else if ( ordering == ROW_MAJOR )
	{
		const float values[ 16 ] = {
			0.f,	0.f,	-1.f,	0.f,
			-1.f,	0.f,	0.f,	0.f,
			0.f,	1.f,	0.f,	0.f,
			0.f,	0.f,	0.f,	1.f,
		};
		changeOfBasisTransform.SetAllValuesAssumingSameOrdering( values );
	}
	return changeOfBasisTransform;
}