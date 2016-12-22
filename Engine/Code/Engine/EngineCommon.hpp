#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Matrix4x4.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/BuildConfig.hpp"

#include "Engine/Core/Logger.hpp"

#if defined ( _DEBUG )
	#define ASSERT_RETURN(e) if (!e) { ASSERT_OR_DIE(e, "ASSERT_RETURN FAILED"); }
#else //#if !defined ( _DEBUG )
	#define ASSERT_RETURN(e) if (!e) { }
#endif

//---------------------------------------------------------------------------------------------
// FIXMEs / TODOs / NOTE macros
// http://flipcode.com/archives/FIXME_TODO_Notes_As_Warnings_In_Compiler_Output.shtml
// Fixes by Chris Forseth
//---------------------------------------------------------------------------------------------

//#define SHOW_TODO //Uncomment to see laundry list of things to revise or add.
#ifdef SHOW_TODO

#define TODO( x )  NOTE( __FILE__LINE__"\n"           \
		" ------------------------------------------------\n" \
		"|  TODO :   " ##x "\n" \
		" -------------------------------------------------\n" )
#define todo( x )  NOTE( __FILE__LINE__" TODO :   " #x "\n" ) 
#else
#define TODO( x ) 
#endif

#define PRAGMA(p)  __pragma( p )
#define NOTE( x )  PRAGMA( message( x ) )
#define FILE_LINE  NOTE( __FILE__LINE__ )
#define _QUOTE(x) # x
#define QUOTE(x) _QUOTE(x)
#define __FILE__LINE__ __FILE__ "(" QUOTE(__LINE__) ") : "
#define FIXME( x )  NOTE(  __FILE__LINE__"\n"           \
		" -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n" \
		"|  FIXME :  " ##x "\n" \
		" -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n" )
#define fixme( x )  NOTE( __FILE__LINE__" FIXME:   " #x "\n" ) 

//-----------------------------------------------------------------------------
#define STATIC //Do-nothing to afford 'static' being used in .cpp's.
#define VIRTUAL
#define UNREFERENCED(x) (void)(x)


//-----------------------------------------------------------------------------
//Master Debug
extern bool g_inDebugMode;


//-----------------------------------------------------------------------------
//Input - Windows Virtual Key Wrappers
#define VK_LBUTTON 0x01
#define VK_RBUTTON 0x02
#define VK_TAB 0x09
#define VK_ENTER 0x0D
#define VK_ESCAPE 0x1B
#define VK_BACKSPACE 0x08
#define VK_SPACE 0x20
#define VK_PAGEUP 0x21
#define VK_PAGEDOWN 0x22
#define VK_END 0x23
#define VK_HOME 0x24
#define VK_LEFT 0x25
#define VK_RIGHT 0x27
#define VK_UP 0x26
#define VK_DOWN 0x28
#define VK_DELETE 0x2E
#define VK_NUMPAD1 0x61
#define VK_NUMPAD2 0x62
#define VK_NUMPAD3 0x63
#define VK_NUMPAD4 0x64
#define VK_NUMPAD5 0x65
#define VK_NUMPAD6 0x66
#define VK_NUMPAD7 0x67
#define VK_NUMPAD8 0x68
#define VK_NUMPAD9 0x69
#define VK_F1 0x70

//-----------------------------------------------------------------------------
//Input - Engine Virtual Key Wrappers
static const char KEY_TO_TOGGLE_DEBUG_INFO = VK_F1;
static const unsigned char KEY_TO_OPEN_CONSOLE = 192; //'~' and '`' do not work!
static const char KEY_TO_TOGGLE_ORIGIN_AXES = 'O';

static const char KEY_TO_MOVE_PILOTED_OBJECT_FORWARD = 'W';
static const char KEY_TO_MOVE_PILOTED_OBJECT_BACKWARD = 'S';
static const char KEY_TO_MOVE_PILOTED_OBJECT_LEFT = 'A';
static const char KEY_TO_MOVE_PILOTED_OBJECT_RIGHT = 'D';
static const char KEY_TO_MOVE_PILOTED_OBJECT_UP = VK_SPACE;
static const char KEY_TO_MOVE_PILOTED_OBJECT_DOWN = 'X';


//-----------------------------------------------------------------------------
//Rendering
enum DebugRenderMode 
{
	RENDER_MODE_NONE, 
	RENDER_MODE_COLORS,
	RENDER_MODE_TEXCOORDS0,
	RENDER_MODE_TEXCOORDS1,
	RENDER_MODE_TEXCOORDS2,
	RENDER_MODE_TEXCOORDS3,
	RENDER_MODE_TANGENTS, 
	RENDER_MODE_BITANGENTS, 
	RENDER_MODE_NORMALS,
	RENDER_MODE_SKINWEIGHTS,
	RENDER_MODE_SKININDICES,
	NUM_DEBUG_RENDER_MODES 
};
extern DebugRenderMode g_currentDebugRenderMode;

static const unsigned int LIGHTS_IN_ENGINE_MAX = 16;
static const Vector4f DEFAULT_BONE_WEIGHTS = Vector4f( 1.f, 0.f, 0.f, 0.f );
static const Vector4<unsigned int> DEFAULT_JOINT_INDICES = Vector4<unsigned int>( 0 );

typedef std::string ResourceID;
typedef int RenderLayerID;
enum VertexGroupingRule { AS_LINES, AS_POINTS, AS_LINE_LOOP, AS_LINE_STRIP, AS_TRIANGLES };
extern unsigned int GetOpenGLVertexGroupingRule( unsigned int engineVertexGroupingRule );

//For AES A1 surface patches' lambdas, but potentially more in the future:
typedef Vector3f( *Expression )( const Vector2f& position, const void* userData );


//-----------------------------------------------------------------------------
// Coordinate System (2D)
typedef Vector2f WorldCoords2D; //May be negative.
static const float SPRITE_ANGLE_CORRECTION = SPRITERENDERER_FORWARD_DIRECTION; //I like my sprites to be as they are when I edit their .png.

static const WorldCoords2D WORLD2D_RIGHT = Vector2f( 1.f, 0.f );
static const WorldCoords2D WORLD2D_LEFT = Vector2f( -1.f, 0.f );
static const WorldCoords2D WORLD2D_UP = Vector2f( 0.f, 1.f );
static const WorldCoords2D WORLD2D_DOWN = Vector2f( 0.f, -1.f );


//-----------------------------------------------------------------------------
// Coordinate System (3D)
typedef Vector3f WorldCoords3D; //May be negative.

static const WorldCoords3D WORLD3D_FORWARD = Vector3f( 1.f, 0.f, 0.f );
static const WorldCoords3D WORLD3D_BACKWARD = Vector3f( -1.f, 0.f, 0.f );
static const WorldCoords3D WORLD3D_LEFT = Vector3f( 0.f, 1.f, 0.f );
static const WorldCoords3D WORLD3D_RIGHT = Vector3f( 0.f, -1.f, 0.f );
static const WorldCoords3D WORLD3D_UP = Vector3f( 0.f, 0.f, 1.f );
static const WorldCoords3D WORLD3D_DOWN = Vector3f( 0.f, 0.f, -1.f );

extern float ENGINE_PERSPECTIVE_FOV_Y_DEGREES;
extern float ENGINE_Z_NEAR;
extern float ENGINE_Z_FAR;
extern Matrix4x4f GetWorldChangeOfBasis( Ordering ordering );

//void GetEngineWorldBasis() { return Matrix4x4( first row/col is right, second up, third forward ) with row/col ordering respectively }


//-----------------------------------------------------------------------------
//Audio
typedef unsigned int SoundID;


//-----------------------------------------------------------------------------
//Memory
extern bool g_showDebugMemoryWindow;
typedef unsigned char byte_t;
typedef int bitfield_int;

//For static maps of resources using const char* instead of std::string (uses an internal new) to simplify memory tracking.
struct StringComparator
{
	bool operator() ( const char* str1, const char* str2 ) const
	{
		return std::strcmp( str1, str2 ) < 0;
	}
};


//--------------------------------------------------------------------------------------------------------------
//Event System
typedef const char* EngineEventID;
