#include "Engine/Renderer/TheRenderer.hpp"

//Oculus SDK, which demands it come first.
#include "Engine/Renderer/RiftUtils.hpp"

#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/FixedBitmapFont.hpp"
#include "Engine/EngineCommon.hpp"
#include "Engine/Renderer/OpenGLExtensions.hpp"

#include <gl/GL.h>
#include <gl/GLU.h>
#pragma comment( lib, "opengl32" ) // Link in the OpenGL32.lib static library
#pragma comment( lib, "GLu32" ) 

#include "Engine/Renderer/OpenGLExtensions.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer/FrameBuffer.hpp"
#include "Engine/Renderer/FrameBufferEffect.hpp"
#include "Engine/Renderer/Light.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/FileUtils/FileUtils.hpp"
#include "Engine/String/StringUtils.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Matrix4x4.hpp"
#include "Engine/Core/TheConsole.hpp"
#include "Engine/Core/Command.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"

//SD3
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer/Framebuffer.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/RenderState.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Input/TheInput.hpp" //For light piloting in UpdateLights.
#include "Engine/Math/Camera3D.hpp" //For light piloting in UpdateLights, VR.
#include "Engine/Math/Camera2D.hpp" //For VR.
#include <memory>

//AES
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/Skeleton.hpp"
#include "Engine/Renderer/AnimationSequence.hpp"


//Minimize the amount of stalls from error checks we do. 
//Could make wrapper around error variable to return a human-readable string rather than look it up.
#define GL_CHECK_ERROR GLCheckError( __FILE__, __LINE__ )
#ifdef OPENGL_CHECK_ERRORS
	GLenum error = glGetError();
	if ( error != GL_NO_ERROR )
		ERROR_AND_DIE( "GL ERROR [%s] at [%s(%i)]", error, file, line )
#endif


		//--------------------------------------------------------------------------------------------------------------
		static const char* defaultVertexShaderSource = "\
#version 410 core\n\
\n\
//Recall, uniforms are 'uniform' across all vertices, where in-variables are unique per-vertex. \n\
uniform mat4 uProj; //Perspective projection because 3D, see sprite renderer for ortho default.\n\
uniform mat4 uView;\n\
uniform mat4 uModel;\n\
\n\
in vec3 inPosition;\n\
in vec2 inUV0;\n\
in vec4 inColor;\n\
\n\
out vec2 passthroughUV0;\n\
out vec4 passthroughTintColor;\n\
\n\
void main()\n\
{\n\
	passthroughUV0 = inUV0;\n\
	passthroughTintColor = inColor;\n\
\n\
	vec4 pos = vec4( inPosition, 1 ); //1 to preserve translation.\n\
	gl_Position = pos * uModel * uView * uProj;\n\
}";


//--------------------------------------------------------------------------------------------------------------
static const char* defaultFragmentShaderSource = "\n\
#version 410 core\n\
\n\
uniform sampler2D uTexDiffuse;\n\
\n\
in vec2 passthroughUV0;\n\
in vec4 passthroughTintColor;\n\
\n\
out vec4 outColor;\n\
\n\
void main()\n\
{\n\
	vec4 diffuseTexColor = texture( uTexDiffuse, passthroughUV0 );\n\
	outColor = diffuseTexColor * passthroughTintColor;\n\
}";


//--------------------------------------------------------------------------------------------------------------
TheRenderer* g_theRenderer = nullptr;

//--------------------------------------------------------------------------------------------------------------
//Class Static State Variables
STATIC const float TheRenderer::DROP_SHADOW_OFFSET = 2.f;
STATIC const unsigned int TheRenderer::DEFAULT_TEXTURE_ID = 1;
STATIC const unsigned int TheRenderer::DEFAULT_SAMPLER_ID = 1;
STATIC const RenderState TheRenderer::DEFAULT_RENDER_STATE_3D = RenderState( CULL_MODE_NONE/*FOR_VR*/, BLEND_MODE_SOURCE_ALPHA, BLEND_MODE_ONE_MINUS_SOURCE_ALPHA, DEPTH_COMPARE_MODE_LESS, true );
STATIC const RenderState TheRenderer::DEFAULT_RENDER_STATE_2D = RenderState( CULL_MODE_BACK, BLEND_MODE_SOURCE_ALPHA, BLEND_MODE_ONE_MINUS_SOURCE_ALPHA, DEPTH_COMPARE_MODE_LESS, false );
STATIC const RenderState TheRenderer::DEFAULT_FBO_RENDER_STATE = RenderState( CULL_MODE_BACK, BLEND_MODE_SOURCE_ALPHA, BLEND_MODE_ONE_MINUS_SOURCE_ALPHA, DEPTH_COMPARE_MODE_LESS, true );
STATIC MeshRenderer* TheRenderer::s_defaultRenderer = nullptr;
STATIC Material* TheRenderer::s_defaultMaterial3D = nullptr;
STATIC Material* TheRenderer::s_defaultMaterial2D = nullptr;
static const VertexDefinition& DEFAULT_RENDERER_VERTEX_DEFINITION = Vertex3D_PCT::DEFINITION;


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::CreateBuiltInDefaults()
{
	byte_t plainWhiteTexel[ 3 ] = { 255, 255, 255 };
	TheRenderer::m_defaultTexture = Texture::CreateTextureFromBytes( "PlainWhite", plainWhiteTexel, Vector2i::ONE, 3 );

	m_defaultProportionalFont = BitmapFont::CreateOrGetFont( "Data/Fonts/Input.fnt" );
	m_defaultMonospaceFont = FixedBitmapFont::CreateOrGetFont( "Data/Fonts/SquirrelFixedFont.png" );

	m_defaultSampler = new Sampler( SamplerFilter::FILTER_NEAREST, SamplerFilter::FILTER_NEAREST, 
									SamplerWrapMode::WRAP_REPEAT, SamplerWrapMode::WRAP_REPEAT );

	Shader* defaultVertexShader = Shader::CreateShaderFromSource( defaultVertexShaderSource, strlen( defaultVertexShaderSource ), ShaderType::VERTEX_SHADER );
	Shader* defaultFragmentShader = Shader::CreateShaderFromSource( defaultFragmentShaderSource, strlen( defaultFragmentShaderSource ), ShaderType::FRAGMENT_SHADER );
	s_defaultMaterial3D = Material::CreateOrGetMaterial( "DefaultMaterial3D", &DEFAULT_RENDER_STATE_3D, &DEFAULT_RENDERER_VERTEX_DEFINITION, "DefaultMaterial3D", defaultVertexShader, defaultFragmentShader );
	s_defaultMaterial2D = Material::CreateOrGetMaterial( "DefaultMaterial2D", &DEFAULT_RENDER_STATE_2D, &DEFAULT_RENDERER_VERTEX_DEFINITION, "DefaultMaterial2D", defaultVertexShader, defaultFragmentShader );
	s_defaultMaterial2D->SetMatrix4x4( "uView", false, &Matrix4x4f::IDENTITY ); //Remember screen space == NDC coordinates.

	s_defaultRenderer = new STATIC MeshRenderer( nullptr, nullptr );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::SetScreenDimensions( double screenWidth, double screenHeight )
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	m_screenWidthAsUnsignedInt = static_cast<unsigned int>( screenWidth );
	m_screenHeightAsUnsignedInt = static_cast<unsigned int>( screenHeight );
}


//--------------------------------------------------------------------------------------------------------------
#pragma region ConsoleCommands
//SD3 A7 (FBO) Console Command State Globals
FrameBuffer* TheRenderer::GetCurrentFBO() { return m_lastNonDefaultFBO; }
static bool s_enableCustomFBOs = false;
bool TheRenderer::IsShowingFBOs() { return s_enableCustomFBOs; }
static unsigned int s_currentFboEffectIndex = 0;
static bool s_pauseTimerUniform = false;
static float s_wrappedTimerDurationInSeconds = 1.f;
//SD3 A6 (Lights) Console Command State Globals
static bool s_isPilotingLight = false;
bool TheRenderer::IsPilotingLight() { return s_isPilotingLight; }
static unsigned int s_currentPilotedLightID = 0;
//SD3 A5 (TBN) Console Command State Globals
static int s_showTangents = 0; TODO( "Remove and use latter-AES render debug modes instead." );
static int s_showBitangents = 0;
//SD3 A4 (Blinn-Phong) Console Command State Globals
static int s_useFog = 1;
static int s_useDiffuse = 1;
static int s_useSpecular = 1;
static int s_useAmbientAndDOT3 = 1;
static int s_useEmissive = 1;
static int s_useFalloffForSpecular = 1;
static int s_useFalloffForAmbientAndDOT3 = 1;
//SD3 A3 (Shaders) Console Command State Globals
static int s_useTimeEffect = 1;
static bool s_useOrtho = false;
static bool s_showAxesAtOrigin = true;
bool TheRenderer::IsShowingAxes() { return s_showAxesAtOrigin; }
//AES A7 (Skeletal Animation) Console Command State Globals
static int g_lastAnimationSelected = 0;
static int g_lastSkeletonSelected = 0;
static float g_animationTimer = 0.f;
static bool g_animationPaused = true;
static AnimationMode g_animationMode = ANIMATION_MODE_LOOP;


//--------------------------------------------------------------------------------------------------------------
static void ToggleAxes( Command& )
{
	s_showAxesAtOrigin = !s_showAxesAtOrigin;
}


//--------------------------------------------------------------------------------------------------------------
static void GenerateSurfacePatchByIndex( Command& args )
{
	TODO( "Write syntax parser to convert input string into math expression tree (RPN?)." );

	static const unsigned int NUM_EXPRESSIONS = 20;
	static unsigned int numInvocation = 0;

	int out;

	bool success = args.GetNextInt( &out, 0 );
	if ( !success || out < 0 )
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( Stringf( "Usage: GenerateSurfacePatchByIndex <0-%d>", NUM_EXPRESSIONS - 1 ).c_str() );
		return;
	}

	//Master list of expressions that could be graphed.
#pragma region Expressions
	static const Expression expressions[ NUM_EXPRESSIONS ] = {
		[]( const Vector2f& p, const void* ) {
		return Vector3f( p.x, p.y, 0.f );
	},
		[]( const Vector2f& p, const void* ) {
		return Vector3f( p.x, p.y, sin( p.x*p.y ) );
	},
		[]( const Vector2f& p, const void* ) {
		return Vector3f( p.x, p.y, p.x*pow( p.y, 3 ) - p.y*pow( p.x, 3 ) );
	},
		[]( const Vector2f& p, const void* ) {
		return Vector3f( p.x, p.y, ( p.x*p.x + 3 * p.y*p.y )*exp( -p.x*p.x - p.y*p.y ) );
	},
		[]( const Vector2f& p, const void* ) {
		return Vector3f( p.x, p.y, -1.f / ( p.x*p.x + p.y*p.y ) );
	},
		[]( const Vector2f& p, const void* ) {
		return Vector3f( p.x, p.y, cos( abs( p.x ) + abs( p.y ) ) );
	},
		[]( const Vector2f& p, const void* ) {
		return Vector3f( p.x, p.y, cos( pow( abs( p.x ) + abs( p.y ), 2 ) ) );
	},
		[]( const Vector2f& p, const void* ) {
		return Vector3f( p.x, p.y, sin( floor( p.x ) * p.x ) );
	},
		[]( const Vector2f& p, const void* ) {
		return Vector3f( p.x, p.y, sin( ceil( sin( p.x ) ) * p.x ) );
	},
		[]( const Vector2f& p, const void* ) {
		return Vector3f( p.x, p.y, ( p.x - 0.2f ) * ( sin( 1.f / ( p.x - .2f ) ) + p.x + .8f ) * ( 10.f*pow( p.x - .1f, 2 ) + .9f ) );
	},
		[]( const Vector2f& p, const void* ) {
		return Vector3f( p.x, p.y, sin( fPI*p.x / 5.f ) - tan( p.x * 2.f ) );
	},
		[]( const Vector2f& p, const void* ) {
		return Vector3f( p.x, p.y, GetMin( sin( fPI*p.x - 2.f ), .452f ) );
	},
		[]( const Vector2f& p, const void* ) {
		return Vector3f( p.x, p.y, ( cos( fPI*p.x ) / ( -log( 2.f )*p.x ) ) * ( p.x*p.x - 9.f ) );
	},
		[]( const Vector2f& p, const void* ) {
		return Vector3f( p.x, p.y, p.x * sin( 10.f * p.x ) );
	},
		[]( const Vector2f& p, const void* ) {
		return Vector3f( p.x, p.y, log( p.x ) * cos( 15.f * p.x ) * -log( p.x ) * log( p.x ) );
	},
		[]( const Vector2f& p, const void* ) {
		return Vector3f( p.x, p.y, sin( 954.f * p.x ) - 2.f*cos( p.x ) );
	},
		[]( const Vector2f& p, const void* ) {
		return Vector3f( p.x, p.y, abs( sin( p.x ) / pow( 2.f, ( pow( p.x,p.x ) - .5f*fPI ) / fPI ) ) );
	},
		[]( const Vector2f& p, const void* ) {
		return Vector3f( p.x, p.y, sin( cos( tan( p.x ) ) ) );
	},
		[]( const Vector2f& p, const void* ) {
		return Vector3f( p.x, p.y, sin( 2.f * sin( 2.f * sin( 2.f * sin( p.x ) ) ) ) );
	},
		[]( const Vector2f& p, const void* ) {
		return Vector3f( p.x, p.y, pow( p.x*p.x + pow( p.y, 3 ) - 1.f, 3 ) - p.x*p.x*pow( p.y,3 ) );
	}
	};
#pragma endregion
	static const char* expressionDescriptions[ NUM_EXPRESSIONS ] = {
		"( x, y, 0 )",
		"( x, y, sin(x*y) )",
		"( x, y, xy^3 - yx^3 )",
		"( x, y, ( xx + 3yy )*e^-( xx + yy ) )" ,
		"( x, y, (xx + yy)^-1 )",
		"( x, y, cos(|x| + |y|) )",
		"( x, y, cos( (|x|+|y|)^2 ) )",
		"sin( floor( x )*x )",
		"sin( ceil( sin( x ) )*x )",
		"( ( x - 0.2 )sin( 1 / ( x - 0.2 ) ) + x + 0.8 )( 10( x - 0.1 ) ^ 2 + 0.9 )",
		"sin( pi*x / 5 ) - tan( x * 2 )",
		"min( sin( pi*x - 2 ),0.452 )",
		"( cos( pi*x ) / ( -ln( 2 )*x ) ) * ( xx - 9 )",
		"x*sin( 10 * x )",
		"ln( x )*cos( 15 * x ) * -ln( x ) * ln( x ) ",
		"sin( 954 * x ) - 2 * cos( x )",
		"abs( sin( x ) / pow( 2, ( pow( x,x ) - .5*pi ) / pi ) )",
		"sin( cos( tan( x ) ) )",
		"sin( 2 * sin( 2 * sin( 2 * sin( x ) ) ) )",
		"pow( xx + yyy - 1, 3 ) - xxyyy"
	};

	if ( !success || out >= NUM_EXPRESSIONS )
	{
		g_theConsole->Printf( "Not enough graphing expressions exist." );
		g_theConsole->Printf( Stringf( "Valid indices : 0-%d", NUM_EXPRESSIONS - 1 ).c_str() );
		return;
	}

	Mesh* patchMesh = g_theRenderer->CreateSurfacePatchMesh3D( expressions[ out ], &Vertex3D_PCUTB::DEFINITION, -5.f, 5.f, 100, -5.f, 5.f, 100, true, Rgba::RED );
	Material* patchMaterial = Material::CreateOrGetMaterial( "SurfacePatch", &TheRenderer::DEFAULT_RENDER_STATE_3D, &Vertex3D_PCUTB::DEFINITION, "BlinnPhongTBN_PCUTB" );
		//Use "Passthrough" instead of "BlinnPhongTBN_PCUTB" to see the tint, latter ignores vertex color.

	MeshRenderer::CreateOrGetMeshRenderer( Stringf( "SurfacePatchRenderer%d_%d", out, numInvocation ), std::shared_ptr<Mesh>( patchMesh ), patchMaterial );
	numInvocation++;

	g_theConsole->Printf( Stringf( "Current Graph: ( x, y ) => %s", expressionDescriptions[ out ] ).c_str() );
}


//--------------------------------------------------------------------------------------------------------------
static void UseOrthogonalProjectionForViewMatrix( Command& args )
{
	UNREFERENCED( args );

	s_useOrtho = !s_useOrtho;
}


//--------------------------------------------------------------------------------------------------------------
static void SetLightIntensity( Command& args )
{
	float out;

	bool success = args.GetNextFloat( &out, 0.0f );
	if ( !success )
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SetLightIntensity <0.0-1.0>" );
		return;
	}

	for ( auto& sp : *ShaderProgram::GetRegistry() )
		sp.second->SetFloat( "uLightIntensity", &out );
}


//--------------------------------------------------------------------------------------------------------------
static void SetAmbientColor( Command& args )
{
	Rgba out;

	bool success = args.GetNextColor( &out, Rgba::WHITE );
	if ( !success )
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SetAmbientColor <0-255> <0-255> <0-255> <0-255>" );
		return;
	}

	for ( auto& sp : *ShaderProgram::GetRegistry() )
		sp.second->SetColor( "uAmbientLight", &out );
}


//--------------------------------------------------------------------------------------------------------------
static void SetLightColor( Command& args )
{
	Rgba out;

	bool success = args.GetNextColor( &out, Rgba::WHITE );
	if ( !success )
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SetLightColor <0-255> <0-255> <0-255> <0-255>" );
		return;
	}

	for ( auto& sp : *ShaderProgram::GetRegistry() )
		sp.second->SetColor( "uLightColor", &out );
}


//--------------------------------------------------------------------------------------------------------------
static void ToggleTimerUniformUse( Command& args )
{
	UNREFERENCED( args );

	s_useTimeEffect = ( s_useTimeEffect == 1 ) ? 0 : 1;

	for ( auto& sp : *ShaderProgram::GetRegistry() )
		sp.second->SetInt( "uUseTime", &s_useTimeEffect );
}//--------------------------------------------------------------------------------------------------------------


 //--------------------------------------------------------------------------------------------------------------
static void ToggleDiffuse( Command& args )
{
	UNREFERENCED( args );

	s_useDiffuse = ( s_useDiffuse == 1 ) ? 0 : 1;

	for ( auto& sp : *ShaderProgram::GetRegistry() )
		sp.second->SetInt( "uUseDiffuse", &s_useDiffuse );
}


//--------------------------------------------------------------------------------------------------------------
static void ToggleSpecular( Command& args )
{
	UNREFERENCED( args );

	s_useSpecular = ( s_useSpecular == 1 ) ? 0 : 1;

	for ( auto& sp : *ShaderProgram::GetRegistry() )
		sp.second->SetInt( "uUseSpecular", &s_useSpecular );
}


//--------------------------------------------------------------------------------------------------------------
static void ToggleEmissive( Command& args )
{
	UNREFERENCED( args );

	s_useEmissive = ( s_useEmissive == 1 ) ? 0 : 1;

	for ( auto& sp : *ShaderProgram::GetRegistry() )
		sp.second->SetInt( "uUseEmissive", &s_useEmissive );
}


//--------------------------------------------------------------------------------------------------------------
static void ToggleAmbientAndDOT3( Command& args )
{
	UNREFERENCED( args );

	s_useAmbientAndDOT3 = ( s_useAmbientAndDOT3 == 1 ) ? 0 : 1;

	for ( auto& sp : *ShaderProgram::GetRegistry() )
		sp.second->SetInt( "uUseAmbientAndDOT3", &s_useAmbientAndDOT3 );
}


//--------------------------------------------------------------------------------------------------------------
static void ToggleFog( Command& args )
{
	UNREFERENCED( args );

	s_useFog = ( s_useFog == 1 ) ? 0 : 1;

	for ( auto& sp : *ShaderProgram::GetRegistry() )
		sp.second->SetInt( "uUseFog", &s_useFog );
}


//--------------------------------------------------------------------------------------------------------------
static void ToggleFalloffForSpecular( Command& args )
{
	UNREFERENCED( args );

	s_useFalloffForSpecular = ( s_useFalloffForSpecular == 1 ) ? 0 : 1;

	for ( auto& sp : *ShaderProgram::GetRegistry() )
		sp.second->SetInt( "uUseFalloffForSpecular", &s_useFalloffForSpecular );
}


//--------------------------------------------------------------------------------------------------------------
static void ToggleFalloffForAmbientAndDOT3( Command& args )
{
	UNREFERENCED( args );

	s_useFalloffForAmbientAndDOT3 = ( s_useFalloffForAmbientAndDOT3 == 1 ) ? 0 : 1;

	for ( auto& sp : *ShaderProgram::GetRegistry() )
		sp.second->SetInt( "uUseFalloffForAmbientAndDOT3", &s_useFalloffForAmbientAndDOT3 );
}


//--------------------------------------------------------------------------------------------------------------
static void ShowTangents( Command& args )
{
	UNREFERENCED( args );

	s_showTangents = ( s_showTangents == 1 ) ? 0 : 1;
	s_showBitangents = 0;

	for ( auto& sp : *ShaderProgram::GetRegistry() )
	{
		sp.second->SetInt( "uShowTangent", &s_showTangents );
		sp.second->SetInt( "uShowBitangent", &s_showBitangents );
	}
}


//--------------------------------------------------------------------------------------------------------------
static void ShowBitangents( Command& args )
{
	UNREFERENCED( args );

	s_showBitangents = ( s_showBitangents == 1 ) ? 0 : 1;
	s_showTangents = 0;


	for ( auto& sp : *ShaderProgram::GetRegistry() )
	{
		sp.second->SetInt( "uShowTangent", &s_showTangents );
		sp.second->SetInt( "uShowBitangent", &s_showBitangents );
	}
}


//--------------------------------------------------------------------------------------------------------------
static void ToggleCustomFBOs( Command& args )
{
	UNREFERENCED( args );

	s_enableCustomFBOs = !s_enableCustomFBOs;
	if ( s_enableCustomFBOs )
		g_theConsole->Printf( Stringf( "Current FBO Effect: %s", FramebufferEffect::GetFboEffectRenderer( s_currentFboEffectIndex )->GetMaterialName().c_str() ).c_str() );
}


//--------------------------------------------------------------------------------------------------------------
static void SetFBOsColorMask( Command& args )
{
	Rgba out;

	bool success = args.GetNextColor( &out, Rgba::WHITE );
	if ( !success )
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SetFBOsColorMask <0-255> <0-255> <0-255> <0-255>" );
		return;
	}

	for ( auto& sp : *ShaderProgram::GetRegistry() )
		sp.second->SetColor( "uColorMask", &out );
}


//--------------------------------------------------------------------------------------------------------------
static void SetFBOsHueShift( Command& args )
{
	float out;

	bool success = args.GetNextFloat( &out, 0.f );
	if ( !success || out < 0.f || out > 360.f )
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SetFBOsHueShift <0.f - 360.f>" );
		return;
	}

	for ( auto& sp : *ShaderProgram::GetRegistry() )
		sp.second->SetFloat( "uHueShift", &out );
}


//--------------------------------------------------------------------------------------------------------------
static void SetFBOsCurrentEffectIndex( Command& args )
{
	int out;

	bool success = args.GetNextInt( &out, 0 );
	if ( !success || out < 0 )
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SetFBOsCurrentEffectIndex <unsigned int>" );
		return;
	}

	int numEffects = (int)FramebufferEffect::GetFboEffectRendererRegistry()->size();
	if ( !success || out >= numEffects )
	{
		g_theConsole->Printf( "Not enough post-process effects setup." );
		g_theConsole->Printf( Stringf( "Current number of readied effects : %d", numEffects ).c_str() );
		return;
	}

	s_currentFboEffectIndex = out;
	g_theConsole->Printf( Stringf( "Current FBO Effect: %s", FramebufferEffect::GetFboEffectRenderer( s_currentFboEffectIndex )->GetMaterialName().c_str() ).c_str() );
}


//--------------------------------------------------------------------------------------------------------------
static void TimerUniformPause( Command& args )
{
	UNREFERENCED( args );

	s_pauseTimerUniform = ( s_pauseTimerUniform == 1 ) ? 0 : 1;
}


//--------------------------------------------------------------------------------------------------------------
static void TimerUniformSetWrappingDurationInSeconds( Command& args )
{
	float out;

	bool success = args.GetNextFloat( &out, 0.f );
	if ( !success || out < 0.f )
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: TimerUniformSetWrappingDurationInSeconds <0.f+>" );
		return;
	}

	s_wrappedTimerDurationInSeconds = out;

	for ( auto& sp : *ShaderProgram::GetRegistry() )
		sp.second->SetFloat( "uWrappingTimerDuration", &out );
}


//--------------------------------------------------------------------------------------------------------------
static void SetFBOsAnaglyphOffset( Command& args )
{
	float offsetU;
	float offsetV;

	bool successU = args.GetNextFloat( &offsetU, 0.f );
	bool successV = args.GetNextFloat( &offsetV, 0.f );
	if ( !( successU && successV ) || ( offsetU < -.25f || offsetU > .25f || offsetV < -.25f || offsetV > .25f ) )
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SetAnaglyphOffset <-.25f to .25f> <-.25f to .25f>" );
		return;
	}

	Vector2f offset = Vector2f( offsetU, offsetV );

	for ( auto& sp : *ShaderProgram::GetRegistry() )
		sp.second->SetVector2( "uOffset", &offset );
}


//--------------------------------------------------------------------------------------------------------------
static void SetFBOsToonShaderQuantizeLevel( Command& args )
{
	float out;

	bool success = args.GetNextFloat( &out, 0.f );
	if ( !success )
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SetFBOsToonShaderQuantizeLevel <float>" );
		return;
	}

	for ( auto& sp : *ShaderProgram::GetRegistry() )
		sp.second->SetFloat( "uQuantizationLevel", &out );
}


//--------------------------------------------------------------------------------------------------------------
static void TimerUniformSetCurrentValue( Command& args )
{
	float out;

	bool success = args.GetNextFloat( &out, 0.f );
	if ( !success )
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: TimerUniformSetCurrentValue <float>" );
		return;
	}

	for ( auto& sp : *ShaderProgram::GetRegistry() )
	{
		sp.second->SetFloat( "uWrappingTimer", &out );
		sp.second->SetFloat( "uUnwrappedTimer", &out );
	}
}


//--------------------------------------------------------------------------------------------------------------
static void PilotLight( Command& args )
{
	int out;

	bool success = args.GetNextInt( &out, 0 );
	if ( !success || out >= LIGHTS_IN_ENGINE_MAX )
	{
		s_isPilotingLight = false;
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( Stringf( "Usage: PilotLight < 0 - (#lights in scene - 1) >" ).c_str() );
		return;
	}

	if ( Light::GetLight( out )->GetIsLightOn() == false )
	{
		s_isPilotingLight = false;
		g_theConsole->Printf( "Please ToggleLight to be on prior to piloting." );
		return;
	}

	s_isPilotingLight = true;
	s_currentPilotedLightID = out;
}


//--------------------------------------------------------------------------------------------------------------
static void SetPilotedLightColor( Command& args )
{
	Rgba out;

	if ( !s_isPilotingLight )
	{
		g_theConsole->Printf( "Please enter PilotLight mode first." );
		return;
	}

	bool success = args.GetNextColor( &out, Rgba::WHITE );
	if ( !success )
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: PilotLight <0-255> <0-255> <0-255> <0-255>" );
		return;
	}

	Light::GetLight( s_currentPilotedLightID )->SetColor( out );
}


//--------------------------------------------------------------------------------------------------------------
static void SetPilotedLightDirection( Command& args )
{
	float outX;
	float outY;
	float outZ;

	if ( !s_isPilotingLight )
	{
		g_theConsole->Printf( "Please enter PilotLight mode first." );
		return;
	}

	bool success = args.GetNextFloat( &outX, 0.f );
	if ( !success )
	{
		g_theConsole->Printf( "Failed to parse x-component." );
		g_theConsole->Printf( "Usage: SetPilotedLightDirection <float> <float> <float>" );
		return;
	}

	success = args.GetNextFloat( &outY, 0.f );
	if ( !success )
	{
		g_theConsole->Printf( "Failed to parse y-component." );
		g_theConsole->Printf( "Usage: SetPilotedLightDirection <float> <float> <float>" );
		return;
	}

	success = args.GetNextFloat( &outZ, 0.f );
	if ( !success )
	{
		g_theConsole->Printf( "Failed to parse z-component." );
		g_theConsole->Printf( "Usage: SetPilotedLightDirection <float> <float> <float>" );
		return;
	}

	Light::GetLight( s_currentPilotedLightID )->SetDirection( Vector3f( outX, outY, outZ ) );
}


//--------------------------------------------------------------------------------------------------------------
static void SetPilotedLightMinDistance( Command& args )
{
	float out;

	if ( !s_isPilotingLight )
	{
		g_theConsole->Printf( "Please enter PilotLight mode first." );
		return;
	}

	bool success = args.GetNextFloat( &out, 0.f );
	if ( !success )
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SetPilotedLightMinDistance <float>" );
		return;
	}

	Light::GetLight( s_currentPilotedLightID )->SetMinDistance( out );
}


//--------------------------------------------------------------------------------------------------------------
static void SetPilotedLightMaxDistance( Command& args )
{
	float out;

	if ( !s_isPilotingLight )
	{
		g_theConsole->Printf( "Please enter PilotLight mode first." );
		return;
	}

	bool success = args.GetNextFloat( &out, 0.f );
	if ( !success )
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SetPilotedLightMaxDistance <float>" );
		return;
	}

	Light::GetLight( s_currentPilotedLightID )->SetMaxDistance( out );
}


//--------------------------------------------------------------------------------------------------------------
static void SetPilotedLightPowerAtMinDistance( Command& args )
{
	float out;

	if ( !s_isPilotingLight )
	{
		g_theConsole->Printf( "Please enter PilotLight mode first." );
		return;
	}

	bool success = args.GetNextFloat( &out, 0.f );
	if ( !success )
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SetPilotedLightPowerAtMinDistance <float>" );
		return;
	}

	Light::GetLight( s_currentPilotedLightID )->SetPowerAtMinDistance( out );
}


//--------------------------------------------------------------------------------------------------------------
static void SetPilotedLightPowerAtMaxDistance( Command& args )
{
	float out;

	if ( !s_isPilotingLight )
	{
		g_theConsole->Printf( "Please enter PilotLight mode first." );
		return;
	}

	bool success = args.GetNextFloat( &out, 0.f );
	if ( !success )
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SetPilotedLightPowerAtMaxDistance <float>" );
		return;
	}

	Light::GetLight( s_currentPilotedLightID )->SetPowerAtMaxDistance( out );
}


//--------------------------------------------------------------------------------------------------------------
static void SetPilotedLightInnerAngleCosine( Command& args )
{
	float out;

	if ( !s_isPilotingLight )
	{
		g_theConsole->Printf( "Please enter PilotLight mode first." );
		return;
	}

	bool success = args.GetNextFloat( &out, 0.f );
	if ( !success )
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SetPilotedLightInnerAngleCosine <float>" );
		return;
	}

	Light::GetLight( s_currentPilotedLightID )->SetInnerAngleCosine( out );
}


//--------------------------------------------------------------------------------------------------------------
static void SetPilotedLightOuterAngleCosine( Command& args )
{
	float out;

	if ( !s_isPilotingLight )
	{
		g_theConsole->Printf( "Please enter PilotLight mode first." );
		return;
	}

	bool success = args.GetNextFloat( &out, 0.f );
	if ( !success )
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SetPilotedLightOuterAngleCosine <float>" );
		return;
	}

	Light::GetLight( s_currentPilotedLightID )->SetOuterAngleCosine( out );
}


//--------------------------------------------------------------------------------------------------------------
static void SetPilotedLightPowerInsideInnerAngle( Command& args )
{
	float out;

	if ( !s_isPilotingLight )
	{
		g_theConsole->Printf( "Please enter PilotLight mode first." );
		return;
	}

	bool success = args.GetNextFloat( &out, 0.f );
	if ( !success )
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SetPilotedLightPowerInsideInnerAngle <float>" );
		return;
	}

	Light::GetLight( s_currentPilotedLightID )->SetPowerInsideInnerAngleCosine( out );
}


//--------------------------------------------------------------------------------------------------------------
static void SetPilotedLightPowerOutsideOuterAngle( Command& args )
{
	float out;

	if ( !s_isPilotingLight )
	{
		g_theConsole->Printf( "Please enter PilotLight mode first." );
		return;
	}

	bool success = args.GetNextFloat( &out, 0.f );
	if ( !success )
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SetPilotedLightPowerOutsideOuterAngle <float>" );
		return;
	}

	Light::GetLight( s_currentPilotedLightID )->SetPowerOutsideOuterAngleCosine( out );
}


//--------------------------------------------------------------------------------------------------------------
static void SetPilotedLightIsDirectional( Command& args )
{
	int out;

	if ( !s_isPilotingLight )
	{
		g_theConsole->Printf( "Please enter PilotLight mode first." );
		return;
	}

	bool success = args.GetNextInt( &out, 0 );
	if ( !success || ( out != 0 && out != 1 ) )
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SetPilotedLightIsDirectional <0 or 1>" );
		return;
	}

	Light::GetLight( s_currentPilotedLightID )->SetIsDirectional( out );
}


//--------------------------------------------------------------------------------------------------------------
static void AnimationPlay( Command& args )
{
	int outAnimation;
	int outSkeleton;

	bool success = args.GetNextInt( &outAnimation, 0 );
	if ( !success )
	{
		g_theConsole->Printf( "Incorrect arguments. Please supply an animation and optionally a skeleton number to use, in that order." );
		g_theConsole->Printf( "Usage: AnimationPlay <uint> [uint] [(L)oop, (C)lamp, (P)ing-pong]" );
		return;
	}

	if ( args.GetNextInt( &outSkeleton, 0 ) )
		g_lastSkeletonSelected = outSkeleton;

	int numAnimations = g_theRenderer->GetNumAddedAnimations();
	int numSkeletons = g_theRenderer->GetNumAddedSkeletons();

	if ( outAnimation >= numAnimations || outSkeleton >= numSkeletons )
	{
		if ( numAnimations == 0 && numSkeletons == 0 )
		{
			g_theConsole->Printf( "Load an animation AND a skeleton using FBXLoad first." );
			return;
		}
		if ( numAnimations == 0 )
		{
			g_theConsole->Printf( "Load an animation with FBXLoad first." );
			return;
		}
		if ( numSkeletons == 0 )
		{
			g_theConsole->Printf( "Load a skeleton with FBXLoad first." );
			return;
		}
		g_theConsole->Printf( "Array out of range. Supply parameters under below limits." );
		g_theConsole->Printf( "Usage: AnimationPlay <0-%d> [0-%d]", numAnimations - 1, numSkeletons - 1 );
		return;
	}

	char wrapMode = 'L'; //"L" for loop, "C" for clamp, "P" for pingpong.
	args.GetNextChar( &wrapMode, 'L' );
	switch ( wrapMode )
	{
		case 'L': g_animationMode = ANIMATION_MODE_LOOP; break;
		case 'C': g_animationMode = ANIMATION_MODE_CLAMP; break;
		case 'P': g_animationMode = ANIMATION_MODE_PINGPONG; break;
	}

	g_lastSkeletonSelected = outSkeleton;
	g_lastAnimationSelected = outAnimation;
	g_animationTimer = 0.f;
	g_animationPaused = false;
}


//--------------------------------------------------------------------------------------------------------------
static void AnimationTogglePause( Command& /*args*/ )
{
	g_animationPaused = !g_animationPaused;
}

//--------------------------------------------------------------------------------------------------------------
static void SetRenderMode( Command& args )
{
	int out;

	bool success = args.GetNextInt( &out, 0 );
	if ( !success || out < 0 || out >= NUM_DEBUG_RENDER_MODES )
	{
		g_theConsole->Printf( "Incorrect arguments." );
		g_theConsole->Printf( "Usage: SetRenderMode <0-9>" );
		return;
	}

	if ( g_currentDebugRenderMode == (DebugRenderMode)out )
	{

	}

	g_currentDebugRenderMode = (DebugRenderMode)out;
	std::string shaderName = "";

	switch ( g_currentDebugRenderMode )
	{
	case RENDER_MODE_COLORS: shaderName += "PassthroughColor"; break;
	case RENDER_MODE_TEXCOORDS0: shaderName += "PassthroughUV0"; break;
	case RENDER_MODE_TEXCOORDS1: shaderName += "PassthroughUV1"; break;
	case RENDER_MODE_TEXCOORDS2: shaderName += "PassthroughUV2"; break;
	case RENDER_MODE_TEXCOORDS3: shaderName += "PassthroughUV3"; break;
	case RENDER_MODE_TANGENTS: shaderName += "PassthroughTangent"; break;
	case RENDER_MODE_BITANGENTS: shaderName += "PassthroughBitangent"; break;
	case RENDER_MODE_NORMALS: shaderName += "PassthroughNormal"; break;
	case RENDER_MODE_SKINWEIGHTS: shaderName += "SkinDebug_BoneWeights"; break;
	case RENDER_MODE_SKININDICES: shaderName += "SkinDebug_BoneIndices"; break;
	default: shaderName += "Default"; break;
	}
	g_theConsole->Printf( Stringf( "Mode changed to %s", shaderName.c_str() ).c_str() );

	static unsigned int numInvocation = 0;

	for ( auto& mr : *MeshRenderer::GetMeshRendererRegistry() )
	{
		const VertexDefinition* vdefn = mr.second->GetVertexDefinition();
		shaderName = ( shaderName == "Default" ? ShaderProgram::GetDefaultShaderNameForVertexDefinition( vdefn ) : shaderName );
		mr.second->SetMaterial( Material::CreateOrGetMaterial( Stringf( "DebugRenderModeMaterial_%s%d", shaderName.c_str(), ++numInvocation ), &RenderState::DEFAULT, vdefn, shaderName.c_str() ), true );
		continue;
	}
}


//--------------------------------------------------------------------------------------------------------------
static void RegisterConsoleCommands()
{
	//SD3 A2
	g_theConsole->RegisterCommand( "ToggleOriginAxes", ToggleAxes );

	//SD3 A3
	g_theConsole->RegisterCommand( "UseOrthogonalProjectionForViewMatrix", UseOrthogonalProjectionForViewMatrix );

	//SD3 A4
	g_theConsole->RegisterCommand( "ToggleTimeEffect", ToggleTimerUniformUse );
	g_theConsole->RegisterCommand( "ToggleDiffuse", ToggleDiffuse );
	g_theConsole->RegisterCommand( "ToggleSpecular", ToggleSpecular );
	g_theConsole->RegisterCommand( "ToggleAmbientAndDOT3", ToggleAmbientAndDOT3 );
	g_theConsole->RegisterCommand( "ToggleEmissive", ToggleEmissive );
	g_theConsole->RegisterCommand( "ToggleFog", ToggleFog );
	g_theConsole->RegisterCommand( "ToggleFalloffForSpecular", ToggleFalloffForSpecular );
	g_theConsole->RegisterCommand( "ToggleFalloffForAmbientAndDOT3", ToggleFalloffForAmbientAndDOT3 );
	g_theConsole->RegisterCommand( "SetLightIntensity", SetLightIntensity );
	g_theConsole->RegisterCommand( "SetLightColor", SetLightColor );
	g_theConsole->RegisterCommand( "SetAmbientColor", SetAmbientColor );

	//SD3 A5
	g_theConsole->RegisterCommand( "ShowTangents", ShowTangents );
	g_theConsole->RegisterCommand( "ShowBitangents", ShowBitangents );

	//SD3 A6
	g_theConsole->RegisterCommand( "PilotLight", PilotLight );
	g_theConsole->RegisterCommand( "SetPilotedLightColor", SetPilotedLightColor );
	g_theConsole->RegisterCommand( "SetPilotedLightDirection", SetPilotedLightDirection );
	g_theConsole->RegisterCommand( "SetPilotedLightMinDistance", SetPilotedLightMinDistance );
	g_theConsole->RegisterCommand( "SetPilotedLightMaxDistance", SetPilotedLightMaxDistance );
	g_theConsole->RegisterCommand( "SetPilotedLightPowerAtMinDistance", SetPilotedLightPowerAtMinDistance );
	g_theConsole->RegisterCommand( "SetPilotedLightPowerAtMaxDistance", SetPilotedLightPowerAtMaxDistance );
	g_theConsole->RegisterCommand( "SetPilotedLightInnerAngleCosine", SetPilotedLightInnerAngleCosine );
	g_theConsole->RegisterCommand( "SetPilotedLightOuterAngleCosine", SetPilotedLightOuterAngleCosine );
	g_theConsole->RegisterCommand( "SetPilotedLightPowerInsideInnerAngle", SetPilotedLightPowerInsideInnerAngle );
	g_theConsole->RegisterCommand( "SetPilotedLightPowerOutsideOuterAngle", SetPilotedLightPowerOutsideOuterAngle );
	g_theConsole->RegisterCommand( "SetPilotedLightIsDirectional", SetPilotedLightIsDirectional );

	//SD3 A7
	g_theConsole->RegisterCommand( "ToggleFBOs", ToggleCustomFBOs );
	g_theConsole->RegisterCommand( "SetFBOsColorMask", SetFBOsColorMask );
	g_theConsole->RegisterCommand( "SetFBOsHueShift", SetFBOsHueShift );
	g_theConsole->RegisterCommand( "SetFBOsCurrentEffectIndex", SetFBOsCurrentEffectIndex );
	g_theConsole->RegisterCommand( "SetFBOsAnaglyphOffset", SetFBOsAnaglyphOffset );
	g_theConsole->RegisterCommand( "SetFBOsToonShaderQuantizeLevel", SetFBOsToonShaderQuantizeLevel );
	g_theConsole->RegisterCommand( "TimerUniformPause", TimerUniformPause );
	g_theConsole->RegisterCommand( "TimerUniformSetCurrentValue", TimerUniformSetCurrentValue );
	g_theConsole->RegisterCommand( "TimerUniformSetWrappingDurationInSeconds", TimerUniformSetWrappingDurationInSeconds );

	//AES A1
	g_theConsole->RegisterCommand( "GenerateSurfacePatchByIndex", GenerateSurfacePatchByIndex );

	//----

	//SD3 A6
	g_theConsole->RegisterCommand( "ToggleLight", ToggleLight );
	g_theConsole->RegisterCommand( "ToggleLights", ToggleLights );

	//AES A2
	g_theConsole->RegisterCommand( "ClearMeshRenderers", []( Command& ) { MeshRenderer::ClearMeshRenderers(); } );
	g_theConsole->RegisterCommand( "SetRenderMode", SetRenderMode );

	//AES A5
	g_theConsole->RegisterCommand( "AnimationPlay", AnimationPlay );
	g_theConsole->RegisterCommand( "AnimationTogglePause", AnimationTogglePause );
	g_theConsole->RegisterCommand( "ClearSkeletons", []( Command& ) { g_theRenderer->DeleteSkeletons(); } );
	g_theConsole->RegisterCommand( "ClearAnimations", []( Command& ) { g_theRenderer->DeleteAnimations(); } );
}
#pragma endregion


//--------------------------------------------------------------------------------------------------------------
#ifdef PLATFORM_RIFT_CV1
bool TheRenderer::IsRiftVisible() const
{
	return m_riftContext->isVisible;
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::RiftStartup()
{
	//MAY BREAK BECAUSE IT ISN'T AT PROGRAM START LIKE IN DEMO!
	ovrResult result = ovr_Initialize(nullptr);
	ASSERT_OR_DIE(OVR_SUCCESS(result), "Failed to initialize LibOVR.");

	//CURRENTLY SKIPPING THEIR LOGIC TO RETRY IF DISPLAY IS LOST!

	//Declare buffers. TURN INTO MEMBERS.
	m_riftContext = new RiftRenderContext();

	//Create session.
	result = ovr_Create/*Session*/(&m_riftContext->session, &m_riftContext->luid);
	if (!OVR_SUCCESS(result))
		ERROR_RECOVERABLE("Failed to create session, is the HMD not on/connected?");

	//If the luid of the Rift != default adapter's, OpenGL can't render to it.
	ovrGraphicsLuid defaultAdapterLuid = GetDefaultAdapterLuid();
	if (memcmp(&m_riftContext->luid, &defaultAdapterLuid, sizeof(m_riftContext->luid)))
		ERROR_AND_DIE("OpenGL supports only the default graphics adapter.");

	m_riftContext->hmdDesc = ovr_GetHmdDesc(m_riftContext->session);

	//SKIPPING THE SETUP OF THE MIRROR WINDOW, SEEMS TO BE ACCOMPLISHED BY THEAPP ALREADY.

	//Create eye textures' swapchain.
	for (int eye = 0; eye < 2; ++eye)
	{
		ovrSizei idealTextureSize = ovr_GetFovTextureSize(m_riftContext->session, ovrEyeType(eye), m_riftContext->hmdDesc.DefaultEyeFov[eye], 1);
		m_riftContext->eyeRenderTexture[eye] = new RiftTextureBuffer(m_riftContext->session, true, true, idealTextureSize, 1, NULL, 1);
		m_riftContext->eyeDepthBuffer[eye] = new RiftDepthBuffer(m_riftContext->eyeRenderTexture[eye]->GetSize(), 0);

		if (!m_riftContext->eyeRenderTexture[eye]->TextureChain)
			ERROR_AND_DIE("Failed to create eye textures.");
	}

	//Create "mirror" texture swapchain ("mirror" as in what gets displayed to the desktop).
	ovrMirrorTextureDesc desc;
	memset(&desc, 0, sizeof(desc));
	desc.Width = (int)GetScreenWidth(); //Note I'm overriding their half-HMD resolution with same size as normal app.
	desc.Height = (int)GetScreenHeight(); //I think this may facilitate switching between the two better.
	desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;

	result = ovr_CreateMirrorTextureGL(m_riftContext->session, &desc, &m_riftContext->mirrorTexture);
	if (!OVR_SUCCESS(result))
		ERROR_AND_DIE( "Failed to create mirror texture." );

	//Config mirror read buffer.
	GLuint texId;
	ovr_GetMirrorTextureBufferGL(m_riftContext->session, m_riftContext->mirrorTexture, &texId);

	//Create and bind FBO.
	glGenFramebuffers(1, &m_riftContext->mirrorFBO);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_riftContext->mirrorFBO);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
	glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	//Kill V-sync for OVR's compositor.
	//wglSwapIntervalEXT(0);
	TODO( "Not necessary? What was the benefit to this?" );

	//Normally the scene startup went here, hopefully things don't explode from me doing it differently.

	//Set the floor level. FloorLevel here returns tracking poses relative to the height of the floor (e.g. as opposed to the eye height).
	ovr_SetTrackingOriginType(m_riftContext->session, ovrTrackingOrigin_FloorLevel);
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::UpdateEyePoses()
{
	if ( g_theInput->WasKeyPressedOnce('R') )
		ovr_RecenterTrackingOrigin( m_riftContext->session );

	// Call ovr_GetRenderDesc each frame to get the ovrEyeRenderDesc, as the returned values (e.g. HmdToEyeOffset) may change at runtime.
	m_riftContext->eyeRenderDesc[0] = ovr_GetRenderDesc(m_riftContext->session, ovrEye_Left, m_riftContext->hmdDesc.DefaultEyeFov[0]);
	m_riftContext->eyeRenderDesc[1] = ovr_GetRenderDesc(m_riftContext->session, ovrEye_Right, m_riftContext->hmdDesc.DefaultEyeFov[1]);

	//Get eye poses, feeding in correct IPD offset from above two lines.
	ovrVector3f HmdToEyeOffset[2] = { m_riftContext->eyeRenderDesc[0].HmdToEyeOffset, m_riftContext->eyeRenderDesc[1].HmdToEyeOffset };
	ovr_GetEyePoses(m_riftContext->session, m_riftContext->frameIndex, ovrTrue, HmdToEyeOffset, m_riftContext->EyeRenderPose, &m_riftContext->sensorSampleTime);
}


//--------------------------------------------------------------------------------------------------------------
Matrix4x4f TheRenderer::CalcRiftViewMatrixMyBasis( int eye, const Camera2D* camera )
{
	// Get view and projection matrices -- scaling to slow down the mouse.
	const float TURN_RATE_SCALE = -.01f; //Likely culprit of jerking when mouse is moved: test by setting zero, see if that removes snap on rotating cam.
	OVR::Matrix4f rollPitchYaw = OVR::Matrix4f::RotationX( camera->m_orientationDegrees * TURN_RATE_SCALE * 0.f );

	//Swap angles around so rotations will be around the right axes after change of basis happens (per trial and error, so it may not be the fastest possible).
	OVR::Matrix4f finalRollPitchYaw = rollPitchYaw * OVR::Matrix4f( m_riftContext->EyeRenderPose[ eye ].Orientation );
	OVR::WorldAxes riftBasis = OVR::WorldAxes( OVR::AxisDirection::Axis_Right, OVR::AxisDirection::Axis_Up, OVR::AxisDirection::Axis_Out ); //x y -z.
	OVR::WorldAxes rotCorrection = OVR::WorldAxes( OVR::AxisDirection::Axis_Down, OVR::AxisDirection::Axis_In, OVR::AxisDirection::Axis_Left ); //-y z -x.
	OVR::Matrix4f changeRotationBasis = OVR::Matrix4f::AxisConversion( rotCorrection, riftBasis );
	finalRollPitchYaw *= changeRotationBasis;

	//IPD.
	OVR::Vector3f finalUp = finalRollPitchYaw.Transform( OVR::Vector3f( 0, 1, 0 ) );
	OVR::Vector3f finalForward = finalRollPitchYaw.Transform( OVR::Vector3f( 0, 0, -1 ) );
	OVR::Vector3f shiftedEyePos = rollPitchYaw.Transform( m_riftContext->EyeRenderPose[ eye ].Position );
	shiftedEyePos += OVR::Vector3f( camera->m_worldPosition.x, camera->m_worldPosition.y, 50.f ); //Pushes out 50 units on z in 2D (their basis).
		//Do not translate with the Rift.	

	OVR::Matrix4f view = OVR::Matrix4f::LookAtRH( shiftedEyePos, shiftedEyePos + finalForward, finalUp );

	Matrix4x4f engineView = Matrix4x4f( view.M[ 0 ][ 0 ], view.M[ 1 ][ 0 ], view.M[ 2 ][ 0 ], view.M[ 3 ][ 0 ],
										view.M[ 0 ][ 1 ], view.M[ 1 ][ 1 ], view.M[ 2 ][ 1 ], view.M[ 3 ][ 1 ],
										view.M[ 0 ][ 2 ], view.M[ 1 ][ 2 ], view.M[ 2 ][ 2 ], view.M[ 3 ][ 2 ],
										view.M[ 0 ][ 3 ], view.M[ 1 ][ 3 ], view.M[ 2 ][ 3 ], view.M[ 3 ][ 3 ], ROW_MAJOR );

	//Used row-major because they push proj*view in their demo, and am transposing here to get to my engine/shaders' v*MVP*EngineBasis form.
	engineView.SetToTranspose( true );

	return engineView;
}


//--------------------------------------------------------------------------------------------------------------
Matrix4x4f TheRenderer::CalcRiftViewMatrixMyBasis( int eye, const Camera3D* camera )
{
	OVR::Matrix4f view = OVR::Matrix4f::Identity();

	//From [x y z].
	OVR::WorldAxes riftBasis = OVR::WorldAxes( OVR::AxisDirection::Axis_Right, OVR::AxisDirection::Axis_Up, OVR::AxisDirection::Axis_In ); //Oculus: [x y z]. Me: [-y z x].
	//To [-y z x] -- the conversion matrix should be [-y z -x].
//	OVR::WorldAxes basisCorrection = OVR::WorldAxes( OVR::AxisDirection::Axis_Out, OVR::AxisDirection::Axis_Up, OVR::AxisDirection::Axis_Right );
	OVR::WorldAxes basisCorrection = OVR::WorldAxes( OVR::AxisDirection::Axis_Up, OVR::AxisDirection::Axis_In, OVR::AxisDirection::Axis_Right );
	OVR::Matrix4f changeRotationBasis = OVR::Matrix4f::AxisConversion( basisCorrection, riftBasis );
	view *= changeRotationBasis;

	//Convert to this engine's matrix class.
	Matrix4x4f engineView = Matrix4x4f( view.M[ 0 ][ 0 ], view.M[ 1 ][ 0 ], view.M[ 2 ][ 0 ], view.M[ 3 ][ 0 ],
										view.M[ 0 ][ 1 ], view.M[ 1 ][ 1 ], view.M[ 2 ][ 1 ], view.M[ 3 ][ 1 ],
										view.M[ 0 ][ 2 ], view.M[ 1 ][ 2 ], view.M[ 2 ][ 2 ], view.M[ 3 ][ 2 ],
										view.M[ 0 ][ 3 ], view.M[ 1 ][ 3 ], view.M[ 2 ][ 3 ], view.M[ 3 ][ 3 ], ROW_MAJOR );
	//Used row-major because they push proj*view in their demo, and am transposing here to get to my engine/shaders' v*MVP*EngineBasis form.
	engineView.SetToTranspose( true );

	//IPD -- inter-eye ("inter-pupil") distance handling, don't forget the [ eye ] indexing makes that differ per eye:
	Vector3f finalCameraUp = Vector3f::UNIT_Z;
	Vector3f finalCameraForward = Vector3f::UNIT_X * -1.f; //Flipped by proj matrix.
	OVR::Vector3f eyeOffset = m_riftContext->EyeRenderPose[ eye ].Position; //Their basis, hence .yzx on next line.
	Vector3f eyePosInCameraSpace = camera->m_worldPosition + Vector3f( eyeOffset.x, eyeOffset.y, eyeOffset.z );

	Matrix4x4f lookAtTransform( COLUMN_MAJOR );
	lookAtTransform.ClearToLookAtMatrix( eyePosInCameraSpace, eyePosInCameraSpace + finalCameraForward, finalCameraUp, engineView.GetOrdering() );
	engineView = lookAtTransform * engineView;

	OVR::Quatf headRotation = m_riftContext->EyeRenderPose[ eye ].Orientation;
	float yawRadiansAboutTheirY, pitchRadiansAboutTheirX, rollRadiansAboutTheirZ;
	headRotation.GetYawPitchRoll( &yawRadiansAboutTheirY, &pitchRadiansAboutTheirX, &rollRadiansAboutTheirZ ); 
		//Tests verify this works as expected relative to HMD--pitch is looking up/down, yaw looks left/right, roll is rolling your head.

	Matrix4x4f headRotMatrix( COLUMN_MAJOR );
	headRotMatrix.ClearToRotationMatrix_MyBasis( ConvertRadiansToDegrees( yawRadiansAboutTheirY ),
												 -ConvertRadiansToDegrees( pitchRadiansAboutTheirX ),
												 0.f /*ConvertRadiansToDegrees( rollRadiansAboutTheirZ )*/, headRotMatrix.GetOrdering() );
	FIXME( "Correct above method to not induce slanting when roll is nonzero, likely the rotation matrix combo around each axis causing it." );
	engineView = headRotMatrix * engineView;

	return engineView;

//	Matrix4x4f mouseRotationYaw( COLUMN_MAJOR );
//	mouseRotationYaw.ClearToRotationMatrix_MyBasis( camera->m_orientation.m_yawDegrees, camera->m_orientation.m_pitchDegrees, 0.f, mouseRotationYaw.GetOrdering() );
//	return mouseRotationYaw * engineView;
}


//--------------------------------------------------------------------------------------------------------------
Matrix4x4f TheRenderer::CalcRiftPerspProjMatrixMyBasis(int eye)
{
	OVR::Matrix4f proj = ovrMatrix4f_Projection( m_riftContext->hmdDesc.DefaultEyeFov[ eye ], 0.2f, 1000.0f, ovrProjection_None );

	Matrix4x4f engineProj = Matrix4x4f( proj.M[ 0 ][ 0 ], proj.M[ 1 ][ 0 ], proj.M[ 2 ][ 0 ], proj.M[ 3 ][ 0 ],
										proj.M[ 0 ][ 1 ], proj.M[ 1 ][ 1 ], proj.M[ 2 ][ 1 ], proj.M[ 3 ][ 1 ],
										proj.M[ 0 ][ 2 ], proj.M[ 1 ][ 2 ], proj.M[ 2 ][ 2 ], proj.M[ 3 ][ 2 ],
										proj.M[ 0 ][ 3 ], proj.M[ 1 ][ 3 ], proj.M[ 2 ][ 3 ], proj.M[ 3 ][ 3 ], ROW_MAJOR );

	//Used row-major because they push proj*view in their demo, and am transposing here to get to my v*MVP*EngineBasis form.
	engineProj.SetToTranspose( true );
	Matrix4x4f projMyBasis; //My matrix mult currently disallowing in-place multiplications.
	projMyBasis = engineProj * GetWorldChangeOfBasis( COLUMN_MAJOR ); //Since Oculus appears to be in default OpenGL [x y -z], I'm (technically) [-y z x].

	return projMyBasis;
}


//--------------------------------------------------------------------------------------------------------------
Matrix4x4f TheRenderer::CalcRiftOrthoProjMatrixMyBasis(int eye)
{
	OVR::Matrix4f perspProj = ovrMatrix4f_Projection( m_riftContext->hmdDesc.DefaultEyeFov[ eye ], 0.2f, 1000.0f, ovrProjection_None );

	const float METERS_FROM_CAMERA = 10.f;
	float orthoDistance = METERS_FROM_CAMERA;
	OVR::Vector2f orthoScale = OVR::Vector2f( 1.0f ) / OVR::Vector2f( m_riftContext->eyeRenderDesc[ eye ].PixelsPerTanAngleAtCenter );
	orthoScale.x = .05f;
	orthoScale.y = .05f;
	//OVR::Matrix4f proj = ovrMatrix4f_OrthoSubProjection( perspProj, orthoScale, orthoDistance + m_riftContext->eyeRenderDesc[ eye ].HmdToEyeOffset.z, m_riftContext->eyeRenderDesc[ eye ].HmdToEyeOffset.x );
	OVR::Matrix4f proj = ovrMatrix4f_OrthoSubProjection( perspProj, orthoScale, orthoDistance + -m_riftContext->eyeRenderDesc[ eye ].HmdToEyeOffset.x, -m_riftContext->eyeRenderDesc[ eye ].HmdToEyeOffset.y );
	//May need to flip z to be -x and x to be -y (my basis).

	Matrix4x4f engineProj = Matrix4x4f( proj.M[ 0 ][ 0 ], proj.M[ 1 ][ 0 ], proj.M[ 2 ][ 0 ], proj.M[ 3 ][ 0 ],
										proj.M[ 0 ][ 1 ], proj.M[ 1 ][ 1 ], proj.M[ 2 ][ 1 ], proj.M[ 3 ][ 1 ],
										proj.M[ 0 ][ 2 ], proj.M[ 1 ][ 2 ], proj.M[ 2 ][ 2 ], proj.M[ 3 ][ 2 ],
										proj.M[ 0 ][ 3 ], proj.M[ 1 ][ 3 ], proj.M[ 2 ][ 3 ], proj.M[ 3 ][ 3 ], ROW_MAJOR );

	//Used row-major because they push proj*view in their demo, and am transposing here to get to my v*MVP*EngineBasis form.
	engineProj.SetToTranspose( true );
	Matrix4x4f projMyBasis;
	projMyBasis = engineProj * GetWorldChangeOfBasis( COLUMN_MAJOR ); //Since Oculus appears to be in default OpenGL [x y -z], I'm (technically) [-y z x].

	//Likely will need to add a y-inverting 2D basis flip here, since mine is y-up and theirs is y-down. Will see.

	return projMyBasis;
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::UpdateRiftForEye( int eye )
{
	// Switch to eye render target
	m_riftContext->currentEye = eye;
	m_riftContext->eyeRenderTexture[eye]->SetAndClearRenderSurface(m_riftContext->eyeDepthBuffer[eye]);
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::CommitFrameToRift(int eye)
{
	// Avoids an error when calling SetAndClearRenderSurface during next iteration.
	// Without this, during the next while loop iteration SetAndClearRenderSurface
	// would bind a framebuffer with an invalid COLOR_ATTACHMENT0 because the texture ID
	// associated with COLOR_ATTACHMENT0 had been unlocked by calling wglDXUnlockObjectsNV.
	m_riftContext->eyeRenderTexture[eye]->UnsetRenderSurface();

	// Commit changes to the textures so they get picked up frame
	m_riftContext->eyeRenderTexture[eye]->Commit();
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::SubmitFrameToRift()
{
	//UPDATE LAYER

	//Do distortion rendering, present, and flush/sync.
	ovrLayerEyeFov layer;
	layer.Header.Type = ovrLayerType_EyeFov;
	layer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft; //OpenGL's texture flipping.

	for (int eye = 0; eye < 2; ++eye)
	{
		layer.ColorTexture[eye] = m_riftContext->eyeRenderTexture[eye]->TextureChain;
		layer.Viewport[eye] = OVR::Recti(m_riftContext->eyeRenderTexture[eye]->GetSize());
		layer.Fov[eye] = m_riftContext->hmdDesc.DefaultEyeFov[eye];
		layer.RenderPose[eye] = m_riftContext->EyeRenderPose[eye];
		layer.SensorSampleTime = m_riftContext->sensorSampleTime;
	}

	//ACTUAL SUBMIT OF FOV TEXTURE TO HEADSET!
	ovrLayerHeader* layers = &layer.Header;
	ovrResult result = ovr_SubmitFrame(m_riftContext->session, m_riftContext->frameIndex, nullptr, &layers, 1);
	// exit the rendering loop if submit returns an error, original demo retried on ovrError_DisplayLost!
	if (!OVR_SUCCESS(result))
		ERROR_AND_DIE("SubmitFrame returned error.");

	m_riftContext->isVisible = (result == ovrSuccess);

	ovrSessionStatus sessionStatus;
	ovr_GetSessionStatus(m_riftContext->session, &sessionStatus);
	if (sessionStatus.ShouldQuit)
		ERROR_RECOVERABLE("Session Status Should Quit! How to handle?");
	if (sessionStatus.ShouldRecenter)
		ovr_RecenterTrackingOrigin(m_riftContext->session);

	//ACTUAL DISPLAY OF MIRROR TEXTURE TO DESKTOP!
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_riftContext->mirrorFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	GLint w = (int)m_screenWidthAsUnsignedInt;
	GLint h = (int)m_screenHeightAsUnsignedInt;
	glBlitFramebuffer(0, h, w, 0,
					  0, 0, w, h,
					  GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	//SwapBuffers(hDC) here in demo.

	m_riftContext->frameIndex++;
}
#endif


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::PreGameStartup()
{
	RegisterConsoleCommands();

	g_theRenderer->SetAlphaFunc( GL_GREATER, 0.1f );

	InitializeFBOs();

#ifdef PLATFORM_RIFT_CV1
	RiftStartup();
#endif
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::InitializeFBOs()
{
	//Make FBO's quad's mesh.
	AABB2f quadBounds; //Should be full-screen in NDC / normalized device coordinates space, so:
	quadBounds.mins = Vector2f( -1.f, -1.f );
	quadBounds.maxs = Vector2f( 1.f, 1.f );

	Vertex3D_PCUTB quad[ 4 ] = //Needs to be in normalized device coordinates, default OpenGL, NOT my basis (what CreateQuadMesh3D_PCUTB gives).
	{
		Vertex3D_PCUTB( Vector3f( quadBounds.mins.x, quadBounds.mins.y, 0.f ), Vector3f( 1.f,0.f,0.f ), Vector3f( 0.f,1.f,0.f ), Rgba::WHITE, Vector2f( 1.f, 1.f ) ), //Top-left.		//0
		Vertex3D_PCUTB( Vector3f( quadBounds.maxs.x, quadBounds.maxs.y, 0.f ), Vector3f( 1.f,0.f,0.f ), Vector3f( 0.f,1.f,0.f ), Rgba::WHITE, Vector2f( 0.f, 0.f ) ), //Bottom-right.	//1
		Vertex3D_PCUTB( Vector3f( quadBounds.maxs.x, quadBounds.mins.y, 0.f ), Vector3f( 1.f,0.f,0.f ), Vector3f( 0.f,1.f,0.f ), Rgba::WHITE, Vector2f( 0.f, 1.f ) ), //Bottom-left.	//2
		Vertex3D_PCUTB( Vector3f( quadBounds.mins.x, quadBounds.maxs.y, 0.f ), Vector3f( 1.f,0.f,0.f ), Vector3f( 0.f,1.f,0.f ), Rgba::WHITE, Vector2f( 1.f, 0.f ) ) //Top-right.	//3
	};
	unsigned int quadIndices[] =
	{
		2, 1, 0, //Counter-Clockwise, else renders to the back and the quad won't show with backface culling.
		0, 1, 3
	};

	DrawInstruction quadDrawInstructions[] = { DrawInstruction( AS_TRIANGLES, 0, 6, true ) };
	Mesh* quadMesh = new Mesh( BufferUsage::STATIC_DRAW, Vertex3D_PCUTB::DEFINITION, 4, quad, 6, quadIndices, 1, quadDrawInstructions );

	m_defaultFboQuad = std::shared_ptr<Mesh>( quadMesh );

	//Make FBO.
	std::vector<TextureFormat> colorFormats;
	colorFormats.push_back( TextureFormat::TEXTURE_FORMAT_Rgba8 );
	const TextureFormat depthStencilFormat = TextureFormat::TEXTURE_FORMAT_Depth24_Stencil8;

	m_lastNonDefaultFBO = new FrameBuffer( m_screenWidthAsUnsignedInt, m_screenHeightAsUnsignedInt, colorFormats, &depthStencilFormat );

#ifdef RENDER_2D_ON_WORLD_QUAD
	//FBO:
	const unsigned int RENDER2D_QUAD_WIDTH = m_screenWidthAsUnsignedInt;
	const unsigned int RENDER2D_QUAD_HEIGHT = m_screenHeightAsUnsignedInt;
	m_render2DOnWorldQuadFBO = new FrameBuffer( RENDER2D_QUAD_WIDTH, RENDER2D_QUAD_HEIGHT, colorFormats, &depthStencilFormat );
	
	//Material: MAY NEED TO MAKE RENDER STATE NOT WRITE DEPTHS? Has to be PCUTB to match QuadPCUTB mesh. Be aware there's a high chance -y will flip--post.vert does *-1 to counter it, but not default .vert.
	Material* projectedQuadMat;
	Shader* defaultVertexShader = Shader::CreateShaderFromSource( defaultVertexShaderSource, strlen( defaultVertexShaderSource ), ShaderType::VERTEX_SHADER );
	Shader* defaultFragmentShader = Shader::CreateShaderFromSource( defaultFragmentShaderSource, strlen( defaultFragmentShaderSource ), ShaderType::FRAGMENT_SHADER );
	projectedQuadMat = Material::CreateOrGetMaterial( "RENDER_2D_ON_WORLD_QUAD", &TheRenderer::DEFAULT_FBO_RENDER_STATE, &Vertex3D_PCUTB::DEFINITION, "RENDER_2D_ON_WORLD_QUAD", defaultVertexShader, defaultFragmentShader );
	projectedQuadMat->SetMatrix4x4( "uModel", false, &Matrix4x4f::IDENTITY );
	projectedQuadMat->SetMatrix4x4( "uView", false, &Matrix4x4f::IDENTITY ); //Rather than render with a custom 3D camera's view, we want to render fullscreen == uView is Identity. 
		//But then we render its quad with uView = 3D cam and the color target of this FBO as its texture.
	projectedQuadMat->SetMatrix4x4( "uProj", false, &Matrix4x4f::IDENTITY ); //Will need to be orthographic?

	//Currently a outputting 1 color render target, 1 depth-and-stencil render target.
	projectedQuadMat->SetTexture( "uTexDiffuse", m_render2DOnWorldQuadFBO->GetColorTextureID( 0 ) );
	projectedQuadMat->SetSampler( "uTexDiffuse", TheRenderer::DEFAULT_SAMPLER_ID );
	projectedQuadMat->SetTexture( "uTexDepth", m_render2DOnWorldQuadFBO->GetDepthStencilTextureID() );
	projectedQuadMat->SetSampler( "uTexDepth", TheRenderer::DEFAULT_SAMPLER_ID );

	FIXME( "Resize world quad mesh based on...?" );
	m_render2DOnWorldQuadRenderer = new MeshRenderer( m_defaultFboQuad, projectedQuadMat );
#endif

}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::PostGameStartup()
{
	//Set command-toggled uniforms. Needs TheGame::Startup run first to add the shaders, as such.
	for ( auto& sp : *ShaderProgram::GetRegistry() )
	{
		sp.second->SetInt( "uUseTime", &s_useTimeEffect );
		sp.second->SetInt( "uUseFog", &s_useFog );
		sp.second->SetInt( "uUseDiffuse", &s_useDiffuse );
		sp.second->SetInt( "uUseSpecular", &s_useSpecular );
		sp.second->SetInt( "uUseAmbientAndDOT3", &s_useAmbientAndDOT3 );
		sp.second->SetInt( "uUseEmissive", &s_useEmissive );
		sp.second->SetInt( "uUseFalloffForSpecular", &s_useFalloffForSpecular );
		sp.second->SetInt( "uUseFalloffForAmbientAndDOT3", &s_useFalloffForAmbientAndDOT3 );
	}
}


//--------------------------------------------------------------------------------------------------------------
TheRenderer::TheRenderer( double screenWidth, double screenHeight )
	: m_lastNonDefaultFBO( nullptr )
	, m_defaultMonospaceFont( nullptr )
	, m_defaultProportionalFont( nullptr )
	, m_defaultSampler( nullptr )
	, m_defaultTexture( nullptr )
	, m_usingDefaultNullFBO( true )
{
	SetScreenDimensions( screenWidth, screenHeight );

	DebuggerPrintf( "OpenGL Vendor is: %s\n", glGetString( GL_VENDOR ) );
	DebuggerPrintf( "OpenGL Version is: %s\n", glGetString( GL_VERSION ) );
	DebuggerPrintf( "GLSL Version is: %s\n", glGetString( GL_SHADING_LANGUAGE_VERSION ) );

	//Managing VBOs.
	glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress( "glGenBuffers" );
	glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress( "glBindBuffer" );
	glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress( "glBufferData" );
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress( "glDeleteBuffers" );
	
	//Loading a shader.
	glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress( "glCreateShader" );
	glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress( "glShaderSource" );
	glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress( "glCompileShader" );
	glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress( "glGetShaderiv" );
	glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress( "glDeleteShader" );

	//Loading a program.
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress( "glCreateProgram" );
	glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress( "glAttachShader" );
	glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress( "glLinkProgram" );
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress( "glGetProgramiv" );
	glDetachShader = (PFNGLDETACHSHADERPROC)wglGetProcAddress( "glDetachShader" );
	glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress( "glDeleteProgram" );
	
	//Error check.
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress( "glGetShaderInfoLog" );
	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress( "glGetProgramInfoLog" );

	//Vertex array descriptor objects.
	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress( "glGenVertexArrays" );
	glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress( "glDeleteVertexArrays" );
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress( "glBindVertexArray" );
	glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)wglGetProcAddress( "glGetAttribLocation" );
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress( "glEnableVertexAttribArray" );
	glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress( "glDisableVertexAttribArray" );
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress( "glVertexAttribPointer" );
	glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC)wglGetProcAddress( "glVertexAttribIPointer" );

	//Drawing. Already have glDrawArrays.
	glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress( "glUseProgram" );

	//Uniforms.
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress( "glGetUniformLocation" );
	glGetActiveUniform = ( PFNGLGETACTIVEUNIFORMPROC )wglGetProcAddress( "glGetActiveUniform" );
	glUniform1fv = (PFNGLUNIFORM1FVPROC)wglGetProcAddress( "glUniform1fv" );
	glUniform2fv = (PFNGLUNIFORM2FVPROC)wglGetProcAddress( "glUniform2fv" );
	glUniform3fv = (PFNGLUNIFORM3FVPROC)wglGetProcAddress( "glUniform3fv" );
	glUniform4fv = (PFNGLUNIFORM4FVPROC)wglGetProcAddress( "glUniform4fv" );
	glUniform1iv = (PFNGLUNIFORM1IVPROC)wglGetProcAddress( "glUniform1iv" );
	glUniform2iv = (PFNGLUNIFORM2IVPROC)wglGetProcAddress( "glUniform2iv" );
	glUniform3iv = (PFNGLUNIFORM3IVPROC)wglGetProcAddress( "glUniform3iv" );
	glUniform4iv = (PFNGLUNIFORM4IVPROC)wglGetProcAddress( "glUniform4iv" );
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress( "glUniformMatrix4fv" );

	//Samplers and textures.
	glGenSamplers = (PFNGLGENSAMPLERSPROC)wglGetProcAddress( "glGenSamplers" );
	glSamplerParameteri = (PFNGLSAMPLERPARAMETERIPROC)wglGetProcAddress( "glSamplerParameteri" );
	glBindSampler = (PFNGLBINDSAMPLERPROC)wglGetProcAddress( "glBindSampler" );
	glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress( "glActiveTexture" );
	glDeleteSamplers = (PFNGLDELETESAMPLERSPROC)wglGetProcAddress( "glDeleteSamplers" );

	//Frame buffer objects.
	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress( "glGenFramebuffers" );
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress( "glBindFramebuffer" );
	glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC)wglGetProcAddress( "glFramebufferTexture" );
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress( "glCheckFramebufferStatus" );
	glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress( "glDeleteFramebuffers" );
	glDrawBuffers = (PFNGLDRAWBUFFERSPROC)wglGetProcAddress( "glDrawBuffers" );
	glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)wglGetProcAddress( "glBlitFramebuffer" );

	//RiftUtils.hpp.
	glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
	glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress("glFramebufferTexture2D");
	glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)wglGetProcAddress("glFramebufferRenderbuffer");

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable( GL_LINE_SMOOTH );
	glLineWidth( 1.5f );

	CreateBuiltInDefaults(); //Ensure this is first to give defaults the first possible texture IDs.
}


//--------------------------------------------------------------------------------------------------------------
TheRenderer::~TheRenderer()
{
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::ClearScreenToColor( const Rgba& colorToClearTo )
{
	//Only seems to take floats, not uchar values. Better to pack with uchar, but costs to range map back.
	float red = Interpolate( 0.f, 1.f, ( static_cast<float>( colorToClearTo.red ) / 255.f ) );
	float green = Interpolate( 0.f, 1.f, ( static_cast<float>( colorToClearTo.green ) / 255.f ) );
	float blue = Interpolate( 0.f, 1.f, ( static_cast<float>( colorToClearTo.blue ) / 255.f ) );

	glClearColor( red, green, blue, 1.f );

	glClear( GL_COLOR_BUFFER_BIT );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::ClearScreenToColor( float red, float green, float blue )
{
	glClearColor( red, green, blue, 1.f );

	glClear( GL_COLOR_BUFFER_BIT );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::ClearScreenDepthBuffer()
{
	glDepthMask( true );
	glClearDepth( 1.f ); //Says push all pixels to maximum far-away so that any pixel will be closer now, this is normalized 0-1.
	glClear( GL_DEPTH_BUFFER_BIT );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::EnableDepthTesting( bool flagValue )
{
	if ( flagValue ) glEnable( GL_DEPTH_TEST );
	else glDisable( GL_DEPTH_TEST );
}

//--------------------------------------------------------------------------------------------------------------
void TheRenderer::EnableAlphaTesting( bool flagValue )
{
	if ( flagValue ) glEnable( GL_ALPHA_TEST );
	else glDisable( GL_ALPHA_TEST );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::SetAlphaFunc( int alphaComparatorFunction, float alphaComparatorValue )
{
	glAlphaFunc( alphaComparatorFunction, alphaComparatorValue );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::EnableBackfaceCulling( bool flagValue )
{
	if ( flagValue ) glEnable( GL_CULL_FACE );
	else glDisable( GL_CULL_FACE );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawVbo_PCT( unsigned int vboID, int numVerts, VertexGroupingRule vertexGroupingRule )
{
	if ( numVerts == 0 ) return;

	glBindBuffer( GL_ARRAY_BUFFER, vboID );

	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_COLOR_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	glVertexPointer( 3, GL_FLOAT, sizeof( Vertex3D_PCT ), (const GLvoid*)offsetof( Vertex3D_PCT, m_position ) );
	glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( Vertex3D_PCT ), (const GLvoid*)offsetof( Vertex3D_PCT, m_color ) );
	glTexCoordPointer( 2, GL_FLOAT, sizeof( Vertex3D_PCT ), (const GLvoid*)offsetof( Vertex3D_PCT, m_texCoords ) );

	glDrawArrays( GetOpenGLVertexGroupingRule( vertexGroupingRule ), 0, numVerts );

	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );

	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	UnbindTexture();
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::BindFBO( FrameBuffer* fbo, bool overwriteLast /*= true*/ )
{
	if ( ( m_usingDefaultNullFBO && fbo == nullptr ) || ( !m_usingDefaultNullFBO && m_lastNonDefaultFBO == fbo ) )
		return; //Already bound.

	if ( fbo == nullptr )
	{
		m_usingDefaultNullFBO = true;
		glBindFramebuffer( GL_FRAMEBUFFER, NULL ); //Default.
		glViewport( 0, 0, m_screenWidthAsUnsignedInt, m_screenHeightAsUnsignedInt );
	}
	else
	{
		m_usingDefaultNullFBO = false;
		if ( overwriteLast )
			m_lastNonDefaultFBO = fbo; //Else we'd try to set FBO uniforms on the default nullptr FBO.
		glBindFramebuffer( GL_FRAMEBUFFER, fbo->GetFrameBufferID() ); //WARNING: calling this line repeatedly induces rapid flicker. Solve by binding null before/after.
		glViewport( 0, 0, fbo->GetWidth(), fbo->GetHeight() );
		//Says how many pixels we're rendering to.
		//GL buffer doesn't need to == screen size this way.
		//i.e. we want to render to ALL of the FBO.
		//Special applications might not want to, but generally you do.

		GLenum renderTargets[ 32 ]; //Good enough for A7's effects.
		for ( unsigned int renderTargetIndex = 0; renderTargetIndex < fbo->GetNumColorRenderTargets(); renderTargetIndex++ )
			renderTargets[ renderTargetIndex ] = GL_COLOR_ATTACHMENT0 + renderTargetIndex;

		/*
			e.g. in GLSL fragment shader.
			out vec4 color0; //By default assigned index 0 -- matches to GL_COLOR_ATTACHMENT0.
			out vec4 color1; //By default assigned index 1, continue on in order of declaration -- matches to GL_COLOR_ATTACHMENT0 + 1.
			But it need not necessarily be so rigid:
			You can in the above loop just assign to renderTargets[32] array in another order.
			If you have varying amounts of out variables to tie FBOs to, you'll implement a system of hints similar to attributes.
			i.e. before linking the shader program, specify if you want to impose a bind point.
		*/

		glDrawBuffers( fbo->GetNumColorRenderTargets(), renderTargets ); //How many and who do the frag shader output variables get rendered to.
	}
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::CopyFBO( FrameBuffer* sourceFBO, FrameBuffer* targetFBO /*= nullptr*/ )
{
	if ( sourceFBO == nullptr )
		return;

	unsigned int fboID = sourceFBO->GetFrameBufferID();
	glBindFramebuffer( GL_READ_FRAMEBUFFER, fboID );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, ( targetFBO ? targetFBO->GetFrameBufferID() : NULL ) ); //Note NULL is the default back buffer.

	unsigned int readWidth = sourceFBO->GetWidth();
	unsigned int readHeight = sourceFBO->GetHeight();

	unsigned int drawWidth = ( targetFBO ? targetFBO->GetWidth() : m_screenWidthAsUnsignedInt );
	unsigned int drawHeight = ( targetFBO ? targetFBO->GetHeight() : m_screenHeightAsUnsignedInt );

	glBlitFramebuffer( 0, 0, //lower left write buffer pixel
		readWidth, readHeight, //top right write buffer pixel				   
		0, 0, //lower left read buffer pixel
		drawWidth, drawHeight, //top right read buffer pixel
		GL_COLOR_BUFFER_BIT, //defines which channel to copy -- if one FBO has more than another, will take min of these two to copy over.
		GL_NEAREST // sampling rule for how it copies/stretches pixels, GL_NEAREST/GL_LINEAR/etc. when source != target dims.
	);

//	GL_CHECK_ERROR();
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::SetDrawColor( float red, float green, float blue, float opacity )
{
	glColor4f( red, green, blue, opacity );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::SetLineWidth( float newLineWidth )
{
	glLineWidth( newLineWidth );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::SetBlendFunc( int sourceBlend, int destinationBlend )
{
	glBlendFunc( sourceBlend, destinationBlend );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::SetRenderFlag( int flagNameToSet )
{
	glEnable( flagNameToSet );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::SetPointSize( float thickness )
{
	glPointSize( thickness );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::SetOrtho( const Vector2f& bottomLeft, const Vector2f& topRight )
{
	Matrix4x4f ortho( COLUMN_MAJOR );
	ortho.ClearToOrthogonalProjection( bottomLeft.x, topRight.x, bottomLeft.y, topRight.y, -1.f, 1.f, ortho.GetOrdering() );
	s_defaultMaterial2D->SetMatrix4x4( "uProj", false, &ortho ); //Don't transpose. Shader order is v*M*V*P.
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::SetPerspective( float fovDegreesY, float aspect, float nearDist, float farDist )
{
	Matrix4x4f persp( COLUMN_MAJOR );
	persp.ClearToPerspectiveProjection( GetWorldChangeOfBasis( COLUMN_MAJOR ), fovDegreesY, aspect, nearDist, farDist, persp.GetOrdering() );
	for ( const ShaderProgramRegistryPair& shaderPair : *ShaderProgram::GetRegistry() )
		shaderPair.second->SetMatrix4x4( "uProj", false, &persp ); //Does s_defaultMaterial3D->SetMatrix4x4( "uProj", false, &persp );

	//NOTE: shaders with other needs will need to overwrite this value per-frame, e.g. in SetupView2D.
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawPoint( const Vector3f& position, float thickness, const Rgba& color /*= Rgba() */ )
{
	glPointSize( thickness );

	UnbindTexture();

	DrawVertexArray3D_PCT( AS_POINTS, { Vertex3D_PCT( position, color ) }, 1 );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawLine( const Vector2f& startPos, const Vector2f& endPos, 
	const Rgba& startColor /*=Rgba()*/, const Rgba& endColor /*=Rgba()*/, float lineThickness /*= 1.f */ )
{
	glLineWidth( lineThickness );

	UnbindTexture();

	Vector3f startPos3D = Vector3f( startPos.x, startPos.y, 0.f );
	Vector3f endPos3D = Vector3f( endPos.x, endPos.y, 0.f );
	DrawVertexArray2D_PCT( AS_LINES, { Vertex3D_PCT( startPos3D, startColor ), Vertex3D_PCT( endPos3D, endColor ) }, 2 );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawLine( const Vector3f& startPos, const Vector3f& endPos, const Rgba& startColor /*= Rgba()*/, const Rgba& endColor /*= Rgba()*/, float lineThickness /*= 1.f */ )
{
	glLineWidth( lineThickness );

	UnbindTexture();

	DrawVertexArray3D_PCT( AS_LINES, { Vertex3D_PCT( startPos, startColor ), Vertex3D_PCT( endPos, endColor ) }, 2 );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawAABB( const int vertexGroupingRule, const AABB3f& bounds, const Texture& texture, const AABB2f* texCoords, const Rgba& tint /*= Rgba()*/, float lineThickness /*= 1.f*/ )
{
	glEnable( GL_TEXTURE_2D );
	s_defaultMaterial2D->SetTexture( "uTexDiffuse", texture.GetTextureID() );

	glColor4ub( tint.red, tint.green, tint.blue, tint.alphaOpacity );

	glLineWidth( lineThickness );

	Vertex3D_PCT vertexes[24];

#pragma region CubeVertices
	//Bottom.
	TODO( "See if it matters that the first two indices are reversed." );
	vertexes[ 1 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.mins.z ), Vector2f( texCoords[ 0 ].maxs.x, texCoords[ 0 ].maxs.y ), tint );
	vertexes[ 0 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.mins.z ), Vector2f( texCoords[ 0 ].mins.x, texCoords[ 0 ].maxs.y ), tint );
	vertexes[ 2 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.mins.y, bounds.mins.z ), Vector2f( texCoords[ 0 ].maxs.x, texCoords[ 0 ].mins.y ), tint );
	vertexes[ 3 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.mins.z ), Vector2f( texCoords[ 0 ].mins.x, texCoords[ 0 ].mins.y ), tint );

	//Top.
	vertexes[ 4 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.maxs.z ), Vector2f( texCoords[ 1 ].mins.x, texCoords[ 1 ].maxs.y ), tint );
	vertexes[ 5 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.mins.y, bounds.maxs.z ), Vector2f( texCoords[ 1 ].maxs.x, texCoords[ 1 ].maxs.y ), tint );
	vertexes[ 6 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.maxs.z ), Vector2f( texCoords[ 1 ].maxs.x, texCoords[ 1 ].mins.y ), tint );
	vertexes[ 7 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.maxs.z ), Vector2f( texCoords[ 1 ].mins.x, texCoords[ 1 ].mins.y ), tint );

	//Left.
	vertexes[ 8  ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.mins.z ), Vector2f( texCoords[ 2 ].mins.x, texCoords[ 2 ].maxs.y ), tint );
	vertexes[ 9  ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.mins.z ), Vector2f( texCoords[ 2 ].maxs.x, texCoords[ 2 ].maxs.y ), tint );
	vertexes[ 10 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.maxs.z ), Vector2f( texCoords[ 2 ].maxs.x, texCoords[ 2 ].mins.y ), tint );
	vertexes[ 11 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.maxs.z ), Vector2f( texCoords[ 2 ].mins.x, texCoords[ 2 ].mins.y ), tint );

	//Right.
	vertexes[ 12 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.mins.y, bounds.mins.z ), Vector2f( texCoords[ 3 ].mins.x, texCoords[ 3 ].maxs.y ), tint );
	vertexes[ 13 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.mins.z ), Vector2f( texCoords[ 3 ].maxs.x, texCoords[ 3 ].maxs.y ), tint );
	vertexes[ 14 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.maxs.z ), Vector2f( texCoords[ 3 ].maxs.x, texCoords[ 3 ].mins.y ), tint );
	vertexes[ 15 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.mins.y, bounds.maxs.z ), Vector2f( texCoords[ 3 ].mins.x, texCoords[ 3 ].mins.y ), tint );

	//Front.
	vertexes[ 16 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.mins.z ), Vector2f( texCoords[ 4 ].mins.x, texCoords[ 4 ].maxs.y ), tint );
	vertexes[ 17 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.mins.y, bounds.mins.z ), Vector2f( texCoords[ 4 ].maxs.x, texCoords[ 4 ].maxs.y ), tint );
	vertexes[ 18 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.mins.y, bounds.maxs.z ), Vector2f( texCoords[ 4 ].maxs.x, texCoords[ 4 ].mins.y ), tint );
	vertexes[ 19 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.maxs.z ), Vector2f( texCoords[ 4 ].mins.x, texCoords[ 4 ].mins.y ), tint );

	//Back.
	vertexes[ 20 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.mins.z ), Vector2f( texCoords[ 5 ].mins.x, texCoords[ 5 ].maxs.y ), tint );
	vertexes[ 21 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.mins.z ), Vector2f( texCoords[ 5 ].maxs.x, texCoords[ 5 ].maxs.y ), tint );
	vertexes[ 22 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.maxs.z ), Vector2f( texCoords[ 5 ].maxs.x, texCoords[ 5 ].mins.y ), tint );
	vertexes[ 23 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.maxs.z ), Vector2f( texCoords[ 5 ].mins.x, texCoords[ 5 ].mins.y ), tint );
#pragma endregion

	unsigned int cubeIndices[] =
#pragma region CubeIBO //Clockwise
	{
		//Bottom.
		0, 1, 2,
		0, 2, 3,
		//Top.
		0 + 4, 1 + 4, 2 + 4,
		0 + 4, 2 + 4, 3 + 4,
		//Left.
		0 + 4 * 2, 1 + 4 * 2, 2 + 4 * 2,
		0 + 4 * 2, 2 + 4 * 2, 3 + 4 * 2,
		//Right.
		0 + 4 * 3, 1 + 4 * 3, 2 + 4 * 3,
		0 + 4 * 3, 2 + 4 * 3, 3 + 4 * 3,
		//Front.
		0 + 4 * 4, 1 + 4 * 4, 2 + 4 * 4,
		0 + 4 * 4, 2 + 4 * 4, 3 + 4 * 4,
		//Back.
		0 + 4 * 5, 1 + 4 * 5, 2 + 4 * 5,
		0 + 4 * 5, 2 + 4 * 5, 3 + 4 * 5
	};
#pragma endregion

	DrawVertexArray3D_PCT( vertexGroupingRule, vertexes, 24, cubeIndices, 36 );

	UnbindTexture();
}


//----------------------//IMPORTANT: vertices iterated CCW from bottom-left, but texels from TOP-left in OpenGL.
void TheRenderer::DrawAABB( const int vertexGroupingRule, const AABB2f& bounds, const Texture& texture,
	const AABB2f& texCoords /*= AABB2f(0,0,1,1)*/, const Rgba& tint  /*=Rgba()*/, float lineThickness /*= 1.f*/ )
{
	glLineWidth( lineThickness );

	s_defaultMaterial2D->SetTexture( "uTexDiffuse", texture.GetTextureID() );

	//Because 2D, positions given in XY plane. No worrying over 3D basis confusions.
	Vertex3D_PCT vertexes[] =
	{
		Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.maxs.y, 0.f ), Vector2f( texCoords.maxs.x, texCoords.mins.y ), tint ),
		Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.mins.y, 0.f ), Vector2f( texCoords.mins.x, texCoords.maxs.y ), tint ),
		Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.mins.y, 0.f ), Vector2f( texCoords.maxs.x, texCoords.maxs.y ), tint ),
		Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.maxs.y, 0.f ), Vector2f( texCoords.mins.x, texCoords.mins.y ), tint )
	};

	unsigned int quadIndicesCounterClockwise[] =
	{
		0, 1, 2,
		3, 1, 0
	};

	DrawVertexArray2D_PCT( vertexGroupingRule, vertexes, 4, quadIndicesCounterClockwise, 6 );

	UnbindTexture();
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawAABB( const int vertexGroupingRule, const AABB2f& bounds, const Rgba& color /*=Rgba()*/, float lineThickness /*= 1.f */ )
{
	UnbindTexture();
	
	glLineWidth( lineThickness );

	Vertex3D_PCT vertexes[] =
	{
		Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.maxs.y, 0.f ), color ),
		Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.mins.y, 0.f ), color ),
		Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.mins.y, 0.f ), color ),
		Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.maxs.y, 0.f ), color )
	};

	unsigned int quadIndices[] =
	{
		2, 1, 0,
		0, 1, 3
	};

	DrawVertexArray2D_PCT( vertexGroupingRule, vertexes, 4, quadIndices, 6 );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawAABB( const int vertexGroupingRule, const AABB3f& bounds, const Rgba& color /*= Rgba()*/, float lineThickness /*= 1.f */ )
{
	UnbindTexture();

	glLineWidth( lineThickness );


	Vertex3D_PCT vertexes[24];

	//Bottom.
	vertexes[ 0 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.mins.z ), color );
	vertexes[ 1 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.mins.z ), color );
	vertexes[ 2 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.mins.y, bounds.mins.z ), color );
	vertexes[ 3 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.mins.z ), color );
			
	//Top.
	vertexes[ 4 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.maxs.z ), color );
	vertexes[ 5 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.mins.y, bounds.maxs.z ), color );
	vertexes[ 6 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.maxs.z ), color );
	vertexes[ 7 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.maxs.z ), color );
			
	//Left.
	vertexes[ 8 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.mins.z ), color );
	vertexes[ 9 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.mins.z ), color );
	vertexes[ 10 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.maxs.z ), color );
	vertexes[ 11 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.maxs.z ), color );
			
	//Right.
	vertexes[ 12 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.maxs.z ), color );
	vertexes[ 13 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.mins.z ), color );
	vertexes[ 14 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.mins.y, bounds.mins.z ), color );
	vertexes[ 15 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.mins.y, bounds.maxs.z ), color );
			
	//Front.
	vertexes[ 16 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.mins.y, bounds.maxs.z ), color );
	vertexes[ 17 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.maxs.z ), color );
	vertexes[ 18 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.mins.z ), color );
	vertexes[ 19 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.mins.y, bounds.mins.z ), color );
			
	//Back.	
	vertexes[ 20 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.mins.z ), color );
	vertexes[ 21 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.mins.z ), color );
	vertexes[ 22 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.maxs.z ), color );
	vertexes[ 23 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.maxs.z ), color );

	unsigned int cubeIndices[] =
#pragma region CubeIBO //Clockwise
	{
		//Bottom.
		0, 1, 2,
		0, 2, 3,
		//Top.
		0 + 4, 1 + 4, 2 + 4,
		0 + 4, 2 + 4, 3 + 4,
		//Left.
		0 + 4 * 2, 1 + 4 * 2, 2 + 4 * 2,
		0 + 4 * 2, 2 + 4 * 2, 3 + 4 * 2,
		//Right.
		0 + 4 * 3, 1 + 4 * 3, 2 + 4 * 3,
		0 + 4 * 3, 2 + 4 * 3, 3 + 4 * 3,
		//Front.
		0 + 4 * 4, 1 + 4 * 4, 2 + 4 * 4,
		0 + 4 * 4, 2 + 4 * 4, 3 + 4 * 4,
		//Back.
		0 + 4 * 5, 1 + 4 * 5, 2 + 4 * 5,
		0 + 4 * 5, 2 + 4 * 5, 3 + 4 * 5
	};
#pragma endregion

	DrawVertexArray3D_PCT( vertexGroupingRule, vertexes, 24, cubeIndices, 36 ); //Be wary of grouping rule affliction by above ordering, e.g. line loop.
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawShadedAABB( const int vertexGroupingRule, const AABB2f& bounds, const Rgba& topLeftColor /*=Rgba()*/, const Rgba& topRightColor /*=Rgba()*/,
	const Rgba& bottomLeftColor /*=Rgba()*/, const Rgba& bottomRightColor /*=Rgba()*/, float lineThickness /*= 1.f */ )
{
	UnbindTexture();

	glLineWidth( lineThickness );

	//Note that previous color-darkening was caused by not setting materials' uTexDiffuse in UnbindTexture (color was * stone tex instead of PlainWhite tex).
	Vertex3D_PCT vertexes[] =
	{
		Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.maxs.y, 0.f ), bottomRightColor ),
		Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.mins.y, 0.f ), topLeftColor ),
		Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.mins.y, 0.f ), topRightColor ),
		Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.maxs.y, 0.f ), bottomLeftColor )
	};

	unsigned int quadIndices[] =
	{
		2, 1, 0,
		0, 1, 3
	};

	DrawVertexArray2D_PCT( vertexGroupingRule, vertexes, 4, quadIndices, 6 );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawShadedAABB( const int vertexGroupingRule, const AABB3f& bounds, const Rgba& topLeftColor /*= Rgba()*/, const Rgba& topRightColor /*= Rgba()*/, const Rgba& bottomLeftColor /*= Rgba()*/, const Rgba& bottomRightColor /*= Rgba()*/, float lineThickness /*= 1.f */ )
{
	UnbindTexture();

	glLineWidth( lineThickness );

	Vertex3D_PCT vertexes[ 24 ];

	//Bottom.
	vertexes[ 0 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.mins.z ), topRightColor );
	vertexes[ 1 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.mins.z ), bottomRightColor );
	vertexes[ 2 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.mins.y, bounds.mins.z ), bottomLeftColor );
	vertexes[ 3 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.mins.z ), topLeftColor );

	//Top.
	vertexes[ 4 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.maxs.z ), topRightColor );
	vertexes[ 5 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.mins.y, bounds.maxs.z ), bottomRightColor );
	vertexes[ 6 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.maxs.z ), bottomLeftColor );
	vertexes[ 7 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.maxs.z ), topLeftColor );

	//Left.
	vertexes[ 8 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.mins.z ), topRightColor );
	vertexes[ 9 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.mins.z ), bottomRightColor );
	vertexes[ 10 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.maxs.z ), bottomLeftColor );
	vertexes[ 11 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.maxs.z ), topLeftColor );

	//Right.
	vertexes[ 12 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.maxs.z ), topRightColor );
	vertexes[ 13 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.mins.z ), bottomRightColor );
	vertexes[ 14 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.mins.y, bounds.mins.z ), bottomLeftColor );
	vertexes[ 15 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.mins.y, bounds.maxs.z ), topLeftColor );

	//Front.
	vertexes[ 16 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.mins.y, bounds.maxs.z ), topRightColor );
	vertexes[ 17 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.maxs.z ), bottomRightColor );
	vertexes[ 18 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.mins.z ), bottomLeftColor );
	vertexes[ 19 ] = Vertex3D_PCT( Vector3f( bounds.mins.x, bounds.mins.y, bounds.mins.z ), topLeftColor );

	//Back.	
	vertexes[ 20 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.mins.z ), topRightColor );
	vertexes[ 21 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.mins.z ), bottomRightColor );
	vertexes[ 22 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.maxs.z ), bottomLeftColor );
	vertexes[ 23 ] = Vertex3D_PCT( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.maxs.z ), topLeftColor );

	unsigned int cubeIndices[] =
#pragma region CubeIBO //Clockwise
	{
		//Bottom.
		0, 1, 2,
		0, 2, 3,
		//Top.
		0 + 4, 1 + 4, 2 + 4,
		0 + 4, 2 + 4, 3 + 4,
		//Left.
		0 + 4 * 2, 1 + 4 * 2, 2 + 4 * 2,
		0 + 4 * 2, 2 + 4 * 2, 3 + 4 * 2,
		//Right.
		0 + 4 * 3, 1 + 4 * 3, 2 + 4 * 3,
		0 + 4 * 3, 2 + 4 * 3, 3 + 4 * 3,
		//Front.
		0 + 4 * 4, 1 + 4 * 4, 2 + 4 * 4,
		0 + 4 * 4, 2 + 4 * 4, 3 + 4 * 4,
		//Back.
		0 + 4 * 5, 1 + 4 * 5, 2 + 4 * 5,
		0 + 4 * 5, 2 + 4 * 5, 3 + 4 * 5
	};
#pragma endregion

	DrawVertexArray3D_PCT( vertexGroupingRule, vertexes, 24, cubeIndices, 36 ); //Be wary of grouping rule affliction by above ordering, e.g. line loop.
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawQuad( const int vertexGroupingRule, const Vector2f& topLeft, const Vector2f& topRight,
	const Vector2f& bottomRight, const Vector2f& bottomLeft, const Rgba& color /*=Rgba()*/, float lineThickness /*= 1.f */ )
{
	UnbindTexture();

	glLineWidth( lineThickness );

	Vertex3D_PCT vertexes[] =
	{
		Vertex3D_PCT( Vector3f( topRight.x, topRight.y, 0.f ), color ),
		Vertex3D_PCT( Vector3f( bottomLeft.x, bottomLeft.y, 0.f ), color ),
		Vertex3D_PCT( Vector3f( bottomRight.x, bottomRight.y, 0.f ), color ),
		Vertex3D_PCT( Vector3f( topLeft.x, topLeft.y, 0.f ), color )
	};

	unsigned int quadIndicesClockwise[] =
	{
		0, 1, 2,
		3, 1, 0
	};

	DrawVertexArray2D_PCT( vertexGroupingRule, vertexes, 4, quadIndicesClockwise, 6 );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawPolygon( const int vertexGroupingRule, const Vector2f& centerPos, float radius, float numSides, float degreesOffset, const Rgba& color /*=Rgba()*/, float lineThickness /*=1.0f*/ )
{
	std::vector<Vertex3D_PCT> vertexes;

	const float radiansTotal = fTWO_PI;
	const float radiansPerSide = radiansTotal / numSides;

	UnbindTexture();

	glLineWidth( lineThickness );

	for ( float radians = 0.f; radians < radiansTotal; radians += radiansPerSide ) {
		float rotatedRadians = radians + ConvertDegreesToRadians( degreesOffset );
		float x = centerPos.x + ( radius * cos( rotatedRadians ) );
		float y = centerPos.y + ( radius * sin( rotatedRadians ) );
		vertexes.push_back( Vertex3D_PCT( Vector3f( x, y, 0.f ), color ) );
	}

	DrawVertexArray3D_PCT( vertexGroupingRule, vertexes, vertexes.size() );
}


//--------------------------------------------------------------------------------------------------------------
float TheRenderer::CalcTextPxHeight( const std::string& inputText, float scale /*= .25f*/, const BitmapFont* font /*= nullptr*/ )
{
	if ( font == nullptr )
		font = m_defaultProportionalFont;

	float maxHeightPx = -1.f;

	Vector2f cursor = Vector2f::ZERO; //Assuming its the lower-left.

	for ( unsigned int charIndex = 0; charIndex < inputText.size(); charIndex++ )
	{
		char c = inputText[ charIndex ];
		const Glyph* currentGlyph = font->GetGlyphForChar( c ); //Use m_fontGlyphs.

		if ( currentGlyph == nullptr )
			continue; //Skips unsupported chars.

		float quadHeightPx = ( currentGlyph->m_yoffset + currentGlyph->m_height ) * scale ;
		if ( maxHeightPx < quadHeightPx )
			maxHeightPx = quadHeightPx;
	}

	return maxHeightPx;
}


//--------------------------------------------------------------------------------------------------------------
float TheRenderer::CalcTextPxWidthUpToIndex( const std::string& inputText, unsigned int indexExclusive, float scale /*= .25f*/, const BitmapFont* font /*= nullptr*/ )
{
	if ( font == nullptr )
		font = m_defaultProportionalFont;

	Vector2f cursor = Vector2f::ZERO; //Assuming its the lower-left.

	const Glyph* previousGlyph = nullptr; //For kerning check.

	for ( unsigned int charIndex = 0; charIndex < indexExclusive; charIndex++ )
	{
		char c = inputText[ charIndex ];
		const Glyph* currentGlyph = font->GetGlyphForChar( c ); //Use m_fontGlyphs.

		if ( currentGlyph == nullptr )
			continue; //Skips unsupported chars.

		float kerningAmount; //BMFont only does kerning horizontally.
		if ( previousGlyph != nullptr )
		{
			kerningAmount = previousGlyph->GetKerning( currentGlyph->m_id );
			cursor.x += ( kerningAmount * scale );
		}

		cursor.x += ( currentGlyph->m_xadvance * scale ); //Move to next glyph.

		previousGlyph = currentGlyph; //For next kerning check.
	}

	return cursor.x;
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawTextInBox2D( const AABB2f& textboxBounds, const Rgba& textboxColor, const std::string& /*text*/, 
								   int /*alignmentHorizontal*/, int /*alignmentVertical*/, float /*textScale*/ /*= .25f*/, 
								   const BitmapFont* /*font*/ /* nullptr*/, const Rgba& /*tint*/ /*= Rgba()*/, bool /*drawDropShadow*/ /*= true*/, const Rgba& /*shadowColor*/ /*=Rgba::BLACK*/ )
{
	DrawAABB( VertexGroupingRule::AS_TRIANGLES, textboxBounds, textboxColor );

	throw new std::logic_error( "DrawTextInBox2D not yet implemented!" );
	TODO( "DrawTextInBox2D!" );

	/*
2. int alignmentH //<, ==, > 0.
	PREPREND SPACES based on the AABB's width.
	For left-align: do nothing.
	For right-align: prepend (AABB.width - CalcTextWidthPx(text)) in spaces/px.
	For center: halve the amount from right-align, prepend half and append half.
3. int alignmentV
	Do the same, but vertically:
	For top/bottom-align: do nothing, which is default position (probably top-left).
	For center: half the amount from right-align and += it to cursorPosParam.y.
	For bottom/top-align: (AABB.height - sum[Tallest's added together]).
4. Word-wrap on whitespace.
	Check if CalcTextWidthPx(text) > AABB2.GetWidth() and if so, word wrap by Tallest(that line).
	Do this by moving the remaining chars to a new string altogether, repeating the process.
	*/
}


//--------------------------------------------------------------------------------------------------------------
//Draws 2D text at a 3D location onto a 2D plane.
void TheRenderer::DrawTextProportional3D( const Vector3f& lowerLeftOriginPos, const std::string& inputText, const Vector3f& textPlaneUpDir, const Vector3f& textPlaneRightDir, 
										  float scale /*= .25f*/, const BitmapFont* font /*= nullptr*/, const Rgba& tint /*= Rgba()*/, bool drawDropShadow /*= true*/, const Rgba& shadowColor /*=Rgba::BLACK*/ )
{
	EnableBackfaceCulling( false );

	if ( font == nullptr )
		font = m_defaultProportionalFont;

	Vector3f cursor = lowerLeftOriginPos; //Assuming its the lower-left.

	const Glyph* previousGlyph = nullptr; //For kerning check.

	for ( unsigned int charIndex = 0; charIndex < inputText.size(); charIndex++ )
	{
		char c = inputText[ charIndex ];
		const Glyph* currentGlyph = font->GetGlyphForChar( c ); //Use m_fontGlyphs.

		if ( currentGlyph == nullptr )
			continue; //Skips unsupported chars.

		float kerningAmount; //BMFont only does kerning horizontally.
		if ( previousGlyph != nullptr )
		{
			kerningAmount = previousGlyph->GetKerning( currentGlyph->m_id );
			cursor += ( textPlaneRightDir * kerningAmount * scale ); //	+ ( kerning.y * scale );
		}

		const Vector2f offset = Vector2f( static_cast<float>( currentGlyph->m_xoffset ), static_cast<float>( currentGlyph->m_yoffset ) ); //"caching" off is good!
		const Vector2f size = Vector2f( static_cast<float>( currentGlyph->m_width ), static_cast<float>( currentGlyph->m_height ) ); //in px, not uv == normalized texel coordinates

		//Remember: in 2D, y+ is up, x+ is right. But positive yoffset means DOWN, though positive xoffset still means right.
		const Vector3f topLeftCorner = cursor + ( textPlaneRightDir * offset.x * scale ) + ( textPlaneUpDir * -offset.y * scale ); //-?
		const Vector3f bottomLeftCorner = topLeftCorner - ( textPlaneUpDir * size.y * scale );
		const Vector3f bottomRightCorner = bottomLeftCorner + ( textPlaneRightDir * size.x * scale );
		const Vector3f topRightCorner = topLeftCorner + ( textPlaneRightDir * size.x * scale );

		AABB3f renderBounds = AABB3f( bottomLeftCorner, topRightCorner );

		AABB2f texCoords = font->GetTexCoordsForGlyph( currentGlyph->m_id );

		Texture* texture = font->GetFontTexture( currentGlyph->m_page );

		if ( drawDropShadow )
		{
			AABB3f shadowRenderBounds = renderBounds;
			Vector3f shadowOffset = Vector3f( DROP_SHADOW_OFFSET, -DROP_SHADOW_OFFSET, -DROP_SHADOW_OFFSET );
			shadowRenderBounds.mins += shadowOffset;
			shadowRenderBounds.maxs += shadowOffset;
			g_theRenderer->DrawAABB( VertexGroupingRule::AS_TRIANGLES, shadowRenderBounds, *texture, &texCoords, shadowColor );
		}
		DrawAABB( VertexGroupingRule::AS_TRIANGLES, renderBounds, *texture, &texCoords, tint );

		cursor += ( textPlaneRightDir * static_cast<float>( currentGlyph->m_xadvance ) * scale ); //Move to next glyph.

		previousGlyph = currentGlyph; //For next kerning check.
	}
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawTextProportional2D( const Vector2f& lowerLeftOriginPos, const std::string& inputText, float scale /*= .25f*/, const BitmapFont* font /*= nullptr*/, const Rgba& tint /*= Rgba()*/, bool drawDropShadow /*= true*/, const Rgba& shadowColor /*=Rgba::BLACK*/ )
{
	if ( font == nullptr )
		font = m_defaultProportionalFont;

	Vector2f cursor = lowerLeftOriginPos; //Assuming its the lower-left.

	const Glyph* previousGlyph = nullptr; //For kerning check.

	for ( unsigned int charIndex = 0; charIndex < inputText.size(); charIndex++ )
	{
		char c = inputText[ charIndex ];
		const Glyph* currentGlyph = font->GetGlyphForChar( c ); //Use m_fontGlyphs.

		if ( currentGlyph == nullptr )
			continue; //Skips unsupported chars.

		float kerningAmount; //BMFont only does kerning horizontally.
		if ( previousGlyph != nullptr )
		{
			kerningAmount = previousGlyph->GetKerning( currentGlyph->m_id );
			cursor.x += ( kerningAmount * scale ); //	+ ( kerning.y * scale );
		}

		const Vector2f offset = Vector2f( static_cast<float>( currentGlyph->m_xoffset ), static_cast<float>( currentGlyph->m_yoffset ) ); //"caching" off is good!
		const Vector2f size = Vector2f( static_cast<float>( currentGlyph->m_width ), static_cast<float>( currentGlyph->m_height ) ); //in px, not uv == normalized texel coordinates

		//Remember: in 2D, y+ is up, x+ is right. But positive yoffset means DOWN, though positive xoffset still means right.
		const Vector2f topLeftCorner		= Vector2f( cursor.x + offset.x * scale, cursor.y - offset.y * scale );
		const Vector2f bottomLeftCorner		= Vector2f( topLeftCorner.x,			topLeftCorner.y - ( size.y * scale ) );
		const Vector2f bottomRightCorner	= Vector2f( bottomLeftCorner.x + ( size.x * scale ), bottomLeftCorner.y );
		const Vector2f topRightCorner		= Vector2f( topLeftCorner.x + ( size.x * scale ),	topLeftCorner.y );

		AABB2f renderBounds = AABB2f( bottomLeftCorner, topRightCorner );

		AABB2f texCoords = font->GetTexCoordsForGlyph( currentGlyph->m_id );

		Texture* texture = font->GetFontTexture( currentGlyph->m_page );

		if ( drawDropShadow )
		{
			AABB2f shadowRenderBounds = renderBounds;
			Vector2f shadowOffset = Vector2f( DROP_SHADOW_OFFSET, -DROP_SHADOW_OFFSET );
			shadowRenderBounds.mins += shadowOffset;
			shadowRenderBounds.maxs += shadowOffset;
			g_theRenderer->DrawAABB( VertexGroupingRule::AS_TRIANGLES, shadowRenderBounds, *texture, texCoords, shadowColor );
		}
		DrawAABB( VertexGroupingRule::AS_TRIANGLES, renderBounds, *texture, texCoords, tint );

		cursor.x += ( currentGlyph->m_xadvance * scale ); //Move to next glyph.

		previousGlyph = currentGlyph; //For next kerning check.
	}
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawTextMonospaced2D( const Vector2f& originPos, const std::string& inputText, float cellHeight, const Rgba& tint /*= Rgba()*/, const FixedBitmapFont* font /*= nullptr*/, float cellAspect /*= 1.f */, bool drawDropShadow /*= true*/ )
{
	if ( font == nullptr ) 
		font = m_defaultMonospaceFont;

	Vector2f glyphBottomLeft = originPos; //Treated as the upper/lower-left.
	Vector2f cellSize( cellAspect * cellHeight, cellHeight );

	for ( int stringIndex = 0; stringIndex < (int)inputText.size( ); stringIndex++ )
	{
		AABB2f texCoords = font->GetTexCoordsForGlyph( inputText[ stringIndex ] ); //this is returning (topLeftX, topLeftY, bottomRightX, bottomRightY).

		Vector2f glyphTopRight = glyphBottomLeft + cellSize;
		AABB2f renderBounds = AABB2f( glyphBottomLeft, glyphTopRight );
		Texture* fontTexture = font->GetFontTexture();

		if ( drawDropShadow )
		{
			AABB2f shadowRenderBounds = renderBounds;
			Vector2f shadowOffset = Vector2f( DROP_SHADOW_OFFSET, -DROP_SHADOW_OFFSET );
			shadowRenderBounds.mins += shadowOffset;
			shadowRenderBounds.maxs += shadowOffset;
			g_theRenderer->DrawAABB( VertexGroupingRule::AS_TRIANGLES, shadowRenderBounds, *fontTexture, texCoords, Rgba::BLACK );
		}
		g_theRenderer->DrawAABB( VertexGroupingRule::AS_TRIANGLES, renderBounds, *fontTexture, texCoords, tint );

		glyphBottomLeft.x += cellSize.x;
	}
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawAxes( float length, float lineThickness /*=1.f*/, float alphaOpacity /*=1.f*/, bool drawZ /*=false*/ )
{
	UnbindTexture();

	glLineWidth( lineThickness );

	DrawLine( Vector3f( 0.f, 0.f, 0.f ), Vector3f( length, 0.f, 0.f ), Rgba( 1.f, 0.f, 0.f, alphaOpacity ), Rgba( 1.f, 0.f, 0.f, alphaOpacity ), lineThickness );
	DrawLine( Vector3f( 0.f, 0.f, 0.f ), Vector3f( 0.f, length, 0.f ), Rgba( 0.f, 1.f, 0.f, alphaOpacity ), Rgba( 0.f, 1.f, 0.f, alphaOpacity ), lineThickness );
	if ( drawZ ) DrawLine( Vector3f( 0.f, 0.f, 0.f ), Vector3f( 0.f, 0.f, length ), Rgba( 0.f, 0.f, 1.f, alphaOpacity ), Rgba( 0.f, 0.f, 1.f, alphaOpacity ), lineThickness );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawDebugAxes( float length /*= 1.f*/, float lineThickness /*=1.f*/, bool drawZ /*=false*/ )
{
	TODO( "Refactor RenderState so we can edit here, since the below Enable/Disable are overridden by s_defaultMaterial3D renderState." );
//	g_theRenderer->EnableDepthTesting( true );
	g_theRenderer->DrawAxes( length, lineThickness, 1.f, drawZ );

//	g_theRenderer->EnableDepthTesting( false );
//	g_theRenderer->DrawAxes( length, 3.f * lineThickness, .3f, drawZ );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawVertexArray3D_PCT( const int vertexGroupingRule, const std::vector< Vertex3D_PCT >& vertexData, unsigned int numVertices )
{
	DrawVertexArray3D_PCT( vertexGroupingRule, vertexData.data(), numVertices );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawVertexArray3D_PCT( const int vertexGroupingRule, const Vertex3D_PCT* vertexData, unsigned int numVertices )
{
	if ( numVertices == 0 )
		return;

	DrawInstruction drawInstructions[] = { DrawInstruction( (VertexGroupingRule)vertexGroupingRule, 0, numVertices, false ) };
	Mesh* mesh = new Mesh( BufferUsage::STATIC_DRAW, DEFAULT_RENDERER_VERTEX_DEFINITION, numVertices, vertexData, 1, drawInstructions );
	s_defaultRenderer->SetMeshAndMaterial( std::shared_ptr<Mesh>( mesh ), s_defaultMaterial3D );

	s_defaultRenderer->Render();

	UnbindTexture();
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawVertexArray3D_PCT( const int vertexGroupingRule, const std::vector< Vertex3D_PCT >& vertexData, unsigned int numVertices, void* indicesData, unsigned int numIndices )
{
	DrawVertexArray3D_PCT( vertexGroupingRule, vertexData.data(), numVertices, indicesData, numIndices );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawVertexArray3D_PCT( const int vertexGroupingRule, const Vertex3D_PCT* vertexData, unsigned int numVertices, void* indicesData, unsigned int numIndices )
{
	if ( numVertices == 0 )
		return;

	DrawInstruction drawInstructions[] = { DrawInstruction( (VertexGroupingRule)vertexGroupingRule, 0, numIndices, true ) };
	Mesh* mesh = new Mesh( BufferUsage::STATIC_DRAW, DEFAULT_RENDERER_VERTEX_DEFINITION, numVertices, vertexData, numIndices, indicesData, 1, drawInstructions );
	s_defaultRenderer->SetMeshAndMaterial( std::shared_ptr<Mesh>( mesh ), s_defaultMaterial3D );
	
	s_defaultRenderer->Render();

	UnbindTexture();
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawVertexArray2D_PCT( const int vertexGroupingRule, const std::vector< Vertex3D_PCT >& vertexData, unsigned int numVertices )
{
	DrawVertexArray2D_PCT( vertexGroupingRule, vertexData.data(), numVertices );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawVertexArray2D_PCT( const int vertexGroupingRule, const Vertex3D_PCT* vertexData, unsigned int numVertices )
{
	if ( numVertices == 0 )
		return;

	DrawInstruction drawInstructions[] = { DrawInstruction( (VertexGroupingRule)vertexGroupingRule, 0, numVertices, false ) };
	Mesh* mesh = new Mesh( BufferUsage::STATIC_DRAW, DEFAULT_RENDERER_VERTEX_DEFINITION, numVertices, vertexData, 1, drawInstructions );
	s_defaultRenderer->SetMeshAndMaterial( std::shared_ptr<Mesh>( mesh ), s_defaultMaterial2D );


//#ifdef PLATFORM_RIFT_CV1 //Overwrite by adding in offsets based on VR HMD.
//	int eye = g_theRenderer->GetRiftContext()->currentEye;
//	Matrix4x4f ortho = g_theRenderer->CalcRiftOrthoProjMatrixMyBasis( eye );
//	s_defaultMaterial2D->SetMatrix4x4( "uProj", false, &ortho );
//#endif

	s_defaultRenderer->Render();

	UnbindTexture();
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawVertexArray2D_PCT( const int vertexGroupingRule, const std::vector< Vertex3D_PCT >& vertexData, unsigned int numVertices, void* indicesData, unsigned int numIndices )
{
	DrawVertexArray2D_PCT( vertexGroupingRule, vertexData.data(), numVertices, indicesData, numIndices );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawVertexArray2D_PCT( const int vertexGroupingRule, const Vertex3D_PCT* vertexData, unsigned int numVertices, void* indicesData, unsigned int numIndices )
{
	if ( numVertices == 0 )
		return;

	DrawInstruction drawInstructions[] = { DrawInstruction( (VertexGroupingRule)vertexGroupingRule, 0, numIndices, true ) };
	Mesh* mesh = new Mesh( BufferUsage::STATIC_DRAW, DEFAULT_RENDERER_VERTEX_DEFINITION, numVertices, vertexData, numIndices, indicesData, 1, drawInstructions );
	s_defaultRenderer->SetMeshAndMaterial( std::shared_ptr<Mesh>( mesh ), s_defaultMaterial2D );

//#ifdef PLATFORM_RIFT_CV1 //Overwrite by adding in offsets based on VR HMD.
//	int eye = g_theRenderer->GetRiftContext()->currentEye;
//	Matrix4x4f ortho = g_theRenderer->CalcRiftOrthoProjMatrixMyBasis( eye );
//	s_defaultMaterial2D->SetMatrix4x4( "uProj", false, &ortho );
//#endif

	s_defaultRenderer->Render();

	UnbindTexture();
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::BindTexture( const Texture* texture )
{
	glEnable( GL_TEXTURE_2D );

	unsigned int paramTextureID = texture->GetTextureID();

	//Necessary to solve the problem of everything getting darkened in the shader pipeline, watch out for unintended side effects.
	s_defaultMaterial2D->SetTexture( "uTexDiffuse", texture->GetTextureID() );
	s_defaultMaterial3D->SetTexture( "uTexDiffuse", texture->GetTextureID() );
	
	glBindTexture( GL_TEXTURE_2D, paramTextureID );
	m_currentTextureID = paramTextureID;
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::UnbindTexture() //Actually just textured drawing with white 1x1 pixel for texture.
{
	BindTexture( m_defaultTexture );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawAnthonyCloudySphere( Vector3f position, float radius, float numPoints, const Rgba& tint /*= Rgba()*/ )
{
	TODO( "Move the code from SD3's A3's Main_Win32 into here to draw correct UV sphere." );

	//Credit Anthony Cloudy of Laundry Squadron.

	std::vector<Vertex3D_PCT> vertexes;
	Vertex3D_PCT vertex;
	const float halfPi = static_cast<float>( HALF_PI );
	const float radiansPerSide = halfPi / numPoints;

	for ( float phi = -halfPi; phi < halfPi; phi += radiansPerSide )
	{
		for ( float theta = 0.f; theta < TWO_PI; theta += radiansPerSide )
		{
			float x = position.x + ( radius * sin( theta ) * cos( phi ) );
			float y = position.y + ( radius * sin( theta ) * sin( phi ) );
			float z = position.z + ( radius * cos( theta ) );

			vertex.m_color = tint;
			vertex.m_position = Vector3f( x, y, z );
			vertexes.push_back( vertex );
		}
	}
	DrawVertexArray3D_PCT( VertexGroupingRule::AS_LINES, vertexes.data(), vertexes.size() );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawSphereGimbal( const int vertexGroupingRule, const Vector3f& centerPos, float radius, float numSides, const Rgba& tint /*= Rgba()*/, float lineThickness /*= 1.0f*/ )
{
	TODO( "Move the code from SD3's A3's Main_Win32 into here to draw correct UV sphere." );

	UnbindTexture();

	glLineWidth( lineThickness );

	std::vector<Vertex3D_PCT> vertexes;
	
	//Invoke DrawPolygon's logic along various interpolating heights.

	const float degreesTotal = 360.f;
	const float degreesPerSide = degreesTotal / numSides;
	const float degreesOffset = 0.f;
	float x, y, z;

	//Build 2D circle in YZ at currentX.
	for ( float degrees = 0.f; degrees < degreesTotal; degrees += degreesPerSide )
	{
		y = centerPos.y + ( radius * CosDegrees( degrees ) );
		z = centerPos.z + ( radius * SinDegrees( degrees ) );

		if ( degrees != 0.f ) //Else would add an extra initial edge from first vertex to itself.
			vertexes.push_back( Vertex3D_PCT( Vector3f( centerPos.x, y, z ), tint ) );

		vertexes.push_back( Vertex3D_PCT( Vector3f( centerPos.x, y, z ), tint ) ); //For drawing AS_LINES.
	}
	//Connect back to 2D circle's starting vertex.
	y = centerPos.y + ( radius * CosDegrees( 0.f ) );
	z = centerPos.z + ( radius * SinDegrees( 0.f ) );
	vertexes.push_back( Vertex3D_PCT( Vector3f( centerPos.x, y, z ), tint ) );

	//Build 2D circle in XZ at currentY.
	for ( float degrees = 0.f; degrees < degreesTotal; degrees += degreesPerSide )
	{
		x = centerPos.x + ( radius * CosDegrees( degrees ) );
		z = centerPos.z + ( radius * SinDegrees( degrees ) );

		if ( degrees != 0.f ) //Else would add an extra initial edge from first vertex to itself.
			vertexes.push_back( Vertex3D_PCT( Vector3f( x, centerPos.y, z ), tint ) );

		vertexes.push_back( Vertex3D_PCT( Vector3f( x, centerPos.y, z ), tint ) ); //For drawing AS_LINES.
	}
	//Connect back to 2D circle's starting vertex.
	x = centerPos.x + ( radius * CosDegrees( 0.f ) );
	z = centerPos.z + ( radius * SinDegrees( 0.f ) );
	vertexes.push_back( Vertex3D_PCT( Vector3f( x, centerPos.y, z ), tint ) );

	//Build 2D circle in XY at currentZ.
	for ( float degrees = 0.f; degrees < degreesTotal; degrees += degreesPerSide )
	{
		x = centerPos.x + ( radius * CosDegrees( degrees ) );
		y = centerPos.y + ( radius * SinDegrees( degrees ) );

		if ( degrees != 0.f ) //Else would add an extra initial edge from first vertex to itself.
			vertexes.push_back( Vertex3D_PCT( Vector3f( x, y, centerPos.z ), tint ) );

		vertexes.push_back( Vertex3D_PCT( Vector3f( x, y, centerPos.z ), tint ) ); //For drawing AS_LINES.
	}
	//Connect back to 2D circle's starting vertex.
	x = centerPos.x + ( radius * CosDegrees( 0.f ) );
	y = centerPos.y + ( radius * SinDegrees( 0.f ) );
	vertexes.push_back( Vertex3D_PCT( Vector3f( x, y, centerPos.z ), tint ) );

	DrawVertexArray3D_PCT( vertexGroupingRule, vertexes, vertexes.size( ) ); //AS_LINES is the ideal.
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DrawCylinder( const int vertexGroupingRule, const Vector3f& centerPos, float radius, float height, float numSlices, float numSidesPerSlice, const Rgba& tint /*= Rgba()*/, float lineThickness /*= 1.0f*/ )
{
	UnbindTexture();

	glLineWidth( lineThickness );

	std::vector<Vertex3D_PCT> vertexes;
	float heightStep = ceil( 1.f / numSlices );
	float halfHeight = height * 0.5f;

	//Invoke DrawPolygon's logic along various interpolating heights.
	for ( float currentHeight = centerPos.z - halfHeight; currentHeight <= centerPos.z + halfHeight; currentHeight += heightStep )
	{
		const float radiansTotal = fTWO_PI;
		const float radiansPerSide = radiansTotal / numSidesPerSlice;
		const float degreesOffset = 0.f;

		//Build circle at height.
		for ( float radians = 0.f; radians < radiansTotal; radians += radiansPerSide ) {
			float x = centerPos.x + ( radius * cos( radians ) );
			float y = centerPos.y + ( radius * sin( radians ) );
			vertexes.push_back( Vertex3D_PCT( Vector3f( x, y, currentHeight ), Vector2f::ZERO, tint ) );
		}
	}

	DrawVertexArray3D_PCT( vertexGroupingRule, vertexes, vertexes.size( ) );
}


//--------------------------------------------------------------------------------------------------------------
Mesh* TheRenderer::CreateQuadMesh3D_PCUTB( const AABB2f& planarBounds, float depth, const Rgba& tint /*= Rgba::WHITE*/, bool windClockwise /*= true*/ )
{
	//Positions given in NEW basis ( [x,y,z] to [-y,z,x] ) with ADJUSTED texture coords.
	Vertex3D_PCUTB quad[ 4 ] = //REMEMBER TO ALSO SEND[ BI ]TANGENT IN THE ENGINE'S Z-UP BASIS!
	{
		Vertex3D_PCUTB( Vector3f( depth, planarBounds.mins.x, planarBounds.mins.y ), WORLD3D_RIGHT, WORLD3D_UP, tint, Vector2f( 1.f, 1.f ) ), //Top-left.		//0
		Vertex3D_PCUTB( Vector3f( depth, planarBounds.maxs.x, planarBounds.maxs.y ), WORLD3D_RIGHT, WORLD3D_UP, tint, Vector2f( 0.f, 0.f ) ), //Bottom-right.	//1
		Vertex3D_PCUTB( Vector3f( depth, planarBounds.maxs.x, planarBounds.mins.y ), WORLD3D_RIGHT, WORLD3D_UP, tint, Vector2f( 0.f, 1.f ) ), //Bottom-left.	//2
		Vertex3D_PCUTB( Vector3f( depth, planarBounds.mins.x, planarBounds.maxs.y ), WORLD3D_RIGHT, WORLD3D_UP, tint, Vector2f( 1.f, 0.f ) ), //Top-right.	//3
	};
	unsigned int quadIndicesClockwise[] = //Needed by meshes.
	{
		0, 1, 2,
		3, 1, 0
	};
	unsigned int quadIndicesCounterClockwise[] = //Needed by FBOs.
	{
		2, 1, 0,
		0, 1, 3
	};

	DrawInstruction quadDrawInstructions[] = { DrawInstruction( AS_TRIANGLES, 0, 6, true ) };
	return windClockwise ?
		new Mesh( BufferUsage::STATIC_DRAW, Vertex3D_PCUTB::DEFINITION, 4, quad, 6, quadIndicesClockwise, 1, quadDrawInstructions ) :
		new Mesh( BufferUsage::STATIC_DRAW, Vertex3D_PCUTB::DEFINITION, 4, quad, 6, quadIndicesCounterClockwise, 1, quadDrawInstructions ) ;
}


//--------------------------------------------------------------------------------------------------------------
Mesh* TheRenderer::CreateAxisTintedCubeMesh3D_PCUTB( const AABB3f& bounds )
{
	AABB2f texCoords[ 6 ];
	texCoords[ 0 ] = AABB2f( Vector2f::ZERO, Vector2f::ONE );
	texCoords[ 1 ] = AABB2f( Vector2f::ZERO, Vector2f::ONE );
	texCoords[ 2 ] = AABB2f( Vector2f::ZERO, Vector2f::ONE );
	texCoords[ 3 ] = AABB2f( Vector2f::ZERO, Vector2f::ONE );
	texCoords[ 4 ] = AABB2f( Vector2f::ZERO, Vector2f::ONE );
	texCoords[ 5 ] = AABB2f( Vector2f::ZERO, Vector2f::ONE );
	
	//Positions and [bi]tangents given in NEW basis ( [x,y,z] to [-y,z,x] ) with ADJUSTED texture coords.
	Vertex3D_PCUTB cube[ 24 ] =
#pragma region CubeVBO
	{
		//Using clockwise windings for now. Commented out lines were pre-IBO.

		//Bottom face. REMEMBER TO SEND [BI]TANGENT IN THE ENGINE'S Z-UP BASIS!
		Vertex3D_PCUTB( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.mins.z ), WORLD3D_LEFT, WORLD3D_BACKWARD, Rgba::CYAN, Vector2f( texCoords[ 0 ].mins.x, texCoords[ 0 ].maxs.y ) ), //0
		Vertex3D_PCUTB( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.mins.z ), WORLD3D_LEFT, WORLD3D_BACKWARD, Rgba::CYAN, Vector2f( texCoords[ 0 ].maxs.x, texCoords[ 0 ].maxs.y ) ), //1
		Vertex3D_PCUTB( Vector3f( bounds.mins.x, bounds.mins.y, bounds.mins.z ), WORLD3D_LEFT, WORLD3D_BACKWARD, Rgba::CYAN, Vector2f( texCoords[ 0 ].maxs.x, texCoords[ 0 ].mins.y ) ), //2
		Vertex3D_PCUTB( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.mins.z ), WORLD3D_LEFT, WORLD3D_BACKWARD, Rgba::CYAN, Vector2f( texCoords[ 0 ].mins.x, texCoords[ 0 ].mins.y ) ), //3

		//Top face.
		Vertex3D_PCUTB( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.maxs.z ), WORLD3D_RIGHT, WORLD3D_FORWARD, Rgba::BLUE, Vector2f( texCoords[ 1 ].mins.x, texCoords[ 1 ].maxs.y ) ), //0
		Vertex3D_PCUTB( Vector3f( bounds.mins.x, bounds.mins.y, bounds.maxs.z ), WORLD3D_RIGHT, WORLD3D_FORWARD, Rgba::BLUE, Vector2f( texCoords[ 1 ].maxs.x, texCoords[ 1 ].maxs.y ) ), //1
		Vertex3D_PCUTB( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.maxs.z ), WORLD3D_RIGHT, WORLD3D_FORWARD, Rgba::BLUE, Vector2f( texCoords[ 1 ].maxs.x, texCoords[ 1 ].mins.y ) ), //2
		Vertex3D_PCUTB( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.maxs.z ), WORLD3D_RIGHT, WORLD3D_FORWARD, Rgba::BLUE, Vector2f( texCoords[ 1 ].mins.x, texCoords[ 1 ].mins.y ) ), //3

		//Left.
		Vertex3D_PCUTB( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.mins.z ), WORLD3D_BACKWARD, WORLD3D_UP, Rgba::GREEN, Vector2f( texCoords[ 2 ].mins.x, texCoords[ 2 ].maxs.y ) ), //0
		Vertex3D_PCUTB( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.mins.z ), WORLD3D_BACKWARD, WORLD3D_UP, Rgba::GREEN, Vector2f( texCoords[ 2 ].maxs.x, texCoords[ 2 ].maxs.y ) ), //1
		Vertex3D_PCUTB( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.maxs.z ), WORLD3D_BACKWARD, WORLD3D_UP, Rgba::GREEN, Vector2f( texCoords[ 2 ].maxs.x, texCoords[ 2 ].mins.y ) ), //2
		Vertex3D_PCUTB( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.maxs.z ), WORLD3D_BACKWARD, WORLD3D_UP, Rgba::GREEN, Vector2f( texCoords[ 2 ].mins.x, texCoords[ 2 ].mins.y ) ), //3

		//Right.
		Vertex3D_PCUTB( Vector3f( bounds.mins.x, bounds.mins.y, bounds.mins.z ), WORLD3D_FORWARD, WORLD3D_UP, Rgba::GREEN * Rgba::GRAY, Vector2f( texCoords[ 3 ].mins.x, texCoords[ 3 ].maxs.y ) ), //0
		Vertex3D_PCUTB( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.mins.z ), WORLD3D_FORWARD, WORLD3D_UP, Rgba::GREEN * Rgba::GRAY, Vector2f( texCoords[ 3 ].maxs.x, texCoords[ 3 ].maxs.y ) ), //1
		Vertex3D_PCUTB( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.maxs.z ), WORLD3D_FORWARD, WORLD3D_UP, Rgba::GREEN * Rgba::GRAY, Vector2f( texCoords[ 3 ].maxs.x, texCoords[ 3 ].mins.y ) ), //2
		Vertex3D_PCUTB( Vector3f( bounds.mins.x, bounds.mins.y, bounds.maxs.z ), WORLD3D_FORWARD, WORLD3D_UP, Rgba::GREEN * Rgba::GRAY, Vector2f( texCoords[ 3 ].mins.x, texCoords[ 3 ].mins.y ) ), //3

		//Front.
		Vertex3D_PCUTB( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.mins.z ), WORLD3D_RIGHT, WORLD3D_UP, Rgba::RED * Rgba::GRAY, Vector2f( texCoords[ 4 ].mins.x, texCoords[ 4 ].maxs.y ) ), //0
		Vertex3D_PCUTB( Vector3f( bounds.mins.x, bounds.mins.y, bounds.mins.z ), WORLD3D_RIGHT, WORLD3D_UP, Rgba::RED * Rgba::GRAY, Vector2f( texCoords[ 4 ].maxs.x, texCoords[ 4 ].maxs.y ) ), //1
		Vertex3D_PCUTB( Vector3f( bounds.mins.x, bounds.mins.y, bounds.maxs.z ), WORLD3D_RIGHT, WORLD3D_UP, Rgba::RED * Rgba::GRAY, Vector2f( texCoords[ 4 ].maxs.x, texCoords[ 4 ].mins.y ) ), //2
		Vertex3D_PCUTB( Vector3f( bounds.mins.x, bounds.maxs.y, bounds.maxs.z ), WORLD3D_RIGHT, WORLD3D_UP, Rgba::RED * Rgba::GRAY, Vector2f( texCoords[ 4 ].mins.x, texCoords[ 4 ].mins.y ) ), //3

		//Back.
		Vertex3D_PCUTB( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.mins.z ), WORLD3D_LEFT, WORLD3D_UP, Rgba::RED, Vector2f( texCoords[ 5 ].mins.x, texCoords[ 5 ].maxs.y ) ), //0
		Vertex3D_PCUTB( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.mins.z ), WORLD3D_LEFT, WORLD3D_UP, Rgba::RED, Vector2f( texCoords[ 5 ].maxs.x, texCoords[ 5 ].maxs.y ) ), //1
		Vertex3D_PCUTB( Vector3f( bounds.maxs.x, bounds.maxs.y, bounds.maxs.z ), WORLD3D_LEFT, WORLD3D_UP, Rgba::RED, Vector2f( texCoords[ 5 ].maxs.x, texCoords[ 5 ].mins.y ) ), //2
		Vertex3D_PCUTB( Vector3f( bounds.maxs.x, bounds.mins.y, bounds.maxs.z ), WORLD3D_LEFT, WORLD3D_UP, Rgba::RED, Vector2f( texCoords[ 5 ].mins.x, texCoords[ 5 ].mins.y ) ) //3
	};
#pragma endregion
	unsigned int cubeIndices[] =
#pragma region CubeIBO //Clockwise
	{
		//Bottom.
		0, 1, 2,
		0, 2, 3,
		//Top.
		0 + 4, 1 + 4, 2 + 4,
		0 + 4, 2 + 4, 3 + 4,
		//Left.
		0 + 4 * 2, 1 + 4 * 2, 2 + 4 * 2,
		0 + 4 * 2, 2 + 4 * 2, 3 + 4 * 2,
		//Right.
		0 + 4 * 3, 1 + 4 * 3, 2 + 4 * 3,
		0 + 4 * 3, 2 + 4 * 3, 3 + 4 * 3,
		//Front.
		0 + 4 * 4, 1 + 4 * 4, 2 + 4 * 4,
		0 + 4 * 4, 2 + 4 * 4, 3 + 4 * 4,
		//Back.
		0 + 4 * 5, 1 + 4 * 5, 2 + 4 * 5,
		0 + 4 * 5, 2 + 4 * 5, 3 + 4 * 5
	};
#pragma endregion

	DrawInstruction cubeDrawInstructions[] = { DrawInstruction( VertexGroupingRule::AS_TRIANGLES, 0, 36, true ) };
	return new Mesh( BufferUsage::STATIC_DRAW, Vertex3D_PCUTB::DEFINITION, 24, cube, 36, cubeIndices, 1, cubeDrawInstructions );
}


//--------------------------------------------------------------------------------------------------------------
Mesh* TheRenderer::CreateSphereMesh3D_PCUTB( const Vector3f& spherePos, float radiusSizeRho, float numCirclesVertical, float numCirclesHorizontal, const Rgba& tint /*= Rgba::WHITE*/ )
{
	TODO( "Change that sphere theta/phi rotation mixup issue." );

	std::vector< Vertex3D_PCUTB > sphere;
	std::vector< unsigned int > sphereIndices;
	float maxThetaDegrees = 360.f;
	float maxPhiDegrees = 180.f; //Should really be between -90 and 90 tho.
	float thetaStep = maxThetaDegrees / numCirclesHorizontal;
	float phiStep = maxPhiDegrees / numCirclesVertical;

	unsigned int numIterations = static_cast<unsigned int>( numCirclesVertical * numCirclesHorizontal );
	sphere.reserve( numIterations * 4 ); //4 vertices per quad * num iterations
	sphereIndices.reserve( numIterations * 6 ); //6 indices to make 2 triangles of said quad

	//Positions and [bi]tangents given in NEW basis ( [x,y,z] to [-y,z,x] ) with ADJUSTED texture coords.

	//Per iteration, create a quad by grabbing other iterations' values (theta+thetaStep, phi+phiStep), (-,+), (+,-).
#pragma region BallVBO
	unsigned int numIteration = 0;
	for ( float thetaDegrees = 0.f; thetaDegrees < maxThetaDegrees; thetaDegrees += thetaStep )
	{
		float sinThisTheta = SinDegrees( thetaDegrees );
		float cosThisTheta = CosDegrees( thetaDegrees );
		float sinNextTheta = SinDegrees( thetaDegrees + thetaStep );
		float cosNextTheta = CosDegrees( thetaDegrees + thetaStep );

		//--U's--

		float thisU = thetaDegrees / maxThetaDegrees;
		float nextU = ( thetaDegrees + thetaStep ) / maxThetaDegrees;

		for ( float phiDegrees = 0.f; phiDegrees < maxPhiDegrees; phiDegrees += phiStep )
		{
			float sinThisPhi = SinDegrees( phiDegrees );
			float cosThisPhi = CosDegrees( phiDegrees );
			float sinNextPhi = SinDegrees( phiDegrees + phiStep );
			float cosNextPhi = CosDegrees( phiDegrees + phiStep );

			//--V's--

			float thisV = phiDegrees / maxPhiDegrees;
			float nextV = ( phiDegrees + phiStep ) / maxPhiDegrees;

#pragma region Positions
			float currentPosX = radiusSizeRho * sinThisPhi * cosThisTheta;
			float currentPosY = radiusSizeRho * sinThisPhi * sinThisTheta;
			float currentPosZ = radiusSizeRho * cosThisPhi;

			//One over on theta: same phi-longitude.
			float nextPosOnThetaX = radiusSizeRho * sinThisPhi * cosNextTheta;
			float nextPosOnThetaY = radiusSizeRho * sinThisPhi * sinNextTheta;
			float nextPosOnThetaZ = radiusSizeRho * cosThisPhi;


			//One over on phi: same theta-latitude.
			float nextPosOnPhiX = radiusSizeRho * sinNextPhi * cosThisTheta;
			float nextPosOnPhiY = radiusSizeRho * sinNextPhi * sinThisTheta;
			float nextPosOnPhiZ = radiusSizeRho * cosNextPhi;

			//One over on both theta and phi.
			float nextPosOnThetaAndPhiX = radiusSizeRho * sinNextPhi * cosNextTheta;
			float nextPosOnThetaAndPhiY = radiusSizeRho * sinNextPhi * sinNextTheta;
			float nextPosOnThetaAndPhiZ = radiusSizeRho * cosNextPhi;

			//Note the above is PRE-change of basis. Need to go [x,y,z] to [-y,z,x].
			Vector3f currentPos = Vector3f( -currentPosY, currentPosZ, currentPosX );
			Vector3f nextPosOnTheta = Vector3f( -nextPosOnThetaY, nextPosOnThetaZ, nextPosOnThetaX );
			Vector3f nextPosOnPhi = Vector3f( -nextPosOnPhiY, nextPosOnPhiZ, nextPosOnPhiX );
			Vector3f nextPosOnThetaAndPhi = Vector3f( -nextPosOnThetaAndPhiY, nextPosOnThetaAndPhiZ, nextPosOnThetaAndPhiX );

			//Offset the model position by supplied world position (ensure latter is given relative to post-change of basis).
			currentPos += spherePos;
			nextPosOnTheta += spherePos;
			nextPosOnPhi += spherePos;
			nextPosOnThetaAndPhi += spherePos;
#pragma endregion
#pragma region Tangents
			//Being tangents, they == d(pos)/dtheta.
			float currentTangentX = radiusSizeRho * sinThisPhi * -sinThisTheta;
			float currentTangentY = radiusSizeRho * sinThisPhi * cosThisTheta;
			float currentTangentZ = 0.f;

			//One over on theta: same phi-longitude.
			float nextTangentOnThetaX = radiusSizeRho * sinThisPhi * -sinNextTheta;
			float nextTangentOnThetaY = radiusSizeRho * sinThisPhi * cosNextTheta;
			float nextTangentOnThetaZ = 0.f;

			//One over on phi: same theta-latitude.
			float nextTangentOnPhiX = radiusSizeRho * sinNextPhi * -sinThisTheta;
			float nextTangentOnPhiY = radiusSizeRho * sinNextPhi * cosThisTheta;
			float nextTangentOnPhiZ = 0.f;

			//One over on both theta and phi.
			float nextTangentOnThetaAndPhiX = radiusSizeRho * sinNextPhi * -sinNextTheta;
			float nextTangentOnThetaAndPhiY = radiusSizeRho * sinNextPhi * cosNextTheta;
			float nextTangentOnThetaAndPhiZ = 0.f;

			//Note the above is PRE-change of basis. Need to go [x,y,z] to [-y,z,x].
			Vector3f currentTangent = Vector3f( -currentTangentY, currentTangentZ, currentTangentX );
			Vector3f nextTangentOnTheta = Vector3f( -nextTangentOnThetaY, nextTangentOnThetaZ, nextTangentOnThetaX );
			Vector3f nextTangentOnPhi = Vector3f( -nextTangentOnPhiY, nextTangentOnPhiZ, nextTangentOnPhiX );
			Vector3f nextTangentOnThetaAndPhi = Vector3f( -nextTangentOnThetaAndPhiY, nextTangentOnThetaAndPhiZ, nextTangentOnThetaAndPhiX );

#pragma endregion
#pragma region Bitangents
			//Being the other direction's tangent, they == d(pos)/dphi.
			float currentBitangentX = radiusSizeRho * cosThisPhi * cosThisTheta;
			float currentBitangentY = radiusSizeRho * cosThisPhi * sinThisTheta;
			float currentBitangentZ = radiusSizeRho * -sinThisPhi;

			//One over on theta: same phi-longitude.
			float nextBitangentOnThetaX = radiusSizeRho * cosThisPhi * cosNextTheta;
			float nextBitangentOnThetaY = radiusSizeRho * cosThisPhi * sinNextTheta;
			float nextBitangentOnThetaZ = radiusSizeRho * -sinThisPhi;


			//One over on phi: same theta-latitude.
			float nextBitangentOnPhiX = radiusSizeRho * cosNextPhi * cosThisTheta;
			float nextBitangentOnPhiY = radiusSizeRho * cosNextPhi * sinThisTheta;
			float nextBitangentOnPhiZ = radiusSizeRho * -sinNextPhi;

			//One over on both theta and phi.
			float nextBitangentOnThetaAndPhiX = radiusSizeRho * cosNextPhi * cosNextTheta;
			float nextBitangentOnThetaAndPhiY = radiusSizeRho * cosNextPhi * sinNextTheta;
			float nextBitangentOnThetaAndPhiZ = radiusSizeRho * -sinNextPhi;

			//Note the above is PRE-change of basis. Need to go [x,y,z] to [-y,z,x].
			Vector3f currentBitangent = Vector3f( -currentBitangentY, currentBitangentZ, currentBitangentX );
			Vector3f nextBitangentOnTheta = Vector3f( -nextBitangentOnThetaY, nextBitangentOnThetaZ, nextBitangentOnThetaX );
			Vector3f nextBitangentOnPhi = Vector3f( -nextBitangentOnPhiY, nextBitangentOnPhiZ, nextBitangentOnPhiX );
			Vector3f nextBitangentOnThetaAndPhi = Vector3f( -nextBitangentOnThetaAndPhiY, nextBitangentOnThetaAndPhiZ, nextBitangentOnThetaAndPhiX );

#pragma endregion

			//Vertex construction: for tex coords, u goes with theta, v goes with phi, as above.
			Vertex3D_PCUTB currentVertex = Vertex3D_PCUTB( currentPos, currentTangent, currentBitangent, Vector2f( thisU, thisV ), tint ); //0
			Vertex3D_PCUTB nextVertexOnTheta = Vertex3D_PCUTB( nextPosOnTheta, nextTangentOnTheta, nextBitangentOnTheta, Vector2f( nextU, thisV ), tint ); //1
			Vertex3D_PCUTB nextVertexOnPhi = Vertex3D_PCUTB( nextPosOnPhi, nextTangentOnPhi, nextBitangentOnPhi, Vector2f( thisU, nextV ), tint ); //2
			Vertex3D_PCUTB nextVertexOnThetaAndPhi = Vertex3D_PCUTB( nextPosOnThetaAndPhi, nextTangentOnThetaAndPhi, nextBitangentOnThetaAndPhi, Vector2f( nextU, nextV ), tint ); //3

			//Push back into VBO. Could just add that string to IBO.
			sphere.push_back( currentVertex ); //0
			sphere.push_back( nextVertexOnTheta ); //1
			sphere.push_back( nextVertexOnPhi ); //2
			sphere.push_back( nextVertexOnThetaAndPhi ); //3

			//Push back into IBO: the indices used go 0-3, then 4-8, etc. hence the offset by 4*numIteration.
			sphereIndices.push_back( 0 + ( 4 * numIteration ) );
			sphereIndices.push_back( 1 + ( 4 * numIteration ) );
			sphereIndices.push_back( 2 + ( 4 * numIteration ) );
			sphereIndices.push_back( 3 + ( 4 * numIteration ) );
			sphereIndices.push_back( 2 + ( 4 * numIteration ) );
			sphereIndices.push_back( 1 + ( 4 * numIteration ) );

			numIteration++;
		}
	}
#pragma endregion

	DrawInstruction sphereDrawInstructions[] = { DrawInstruction( VertexGroupingRule::AS_TRIANGLES, 0, 6 * numIterations, false ) };
	return new Mesh( BufferUsage::STATIC_DRAW, Vertex3D_PCUTB::DEFINITION, sphere.size(), sphere.data(), sphereIndices.size(), sphereIndices.data(), 1, sphereDrawInstructions );
}


//--------------------------------------------------------------------------------------------------------------
MeshRenderer* TheRenderer::CreateQuadRenderer3D_PCUTB( const AABB2f& planarBounds, float depth, const Rgba& tint, const std::string& shaderProgramName, const RenderState& renderState )
{
	static unsigned int numInvocation = 0;

	Mesh* quadMesh = CreateQuadMesh3D_PCUTB( planarBounds, depth, tint );

	std::string name = "QuadTBN_" + shaderProgramName;
	Material* quadMaterial = Material::CreateOrGetMaterial( name, &renderState, &Vertex3D_PCUTB::DEFINITION, shaderProgramName.c_str() );

	return MeshRenderer::CreateOrGetMeshRenderer( Stringf( "%s%d", name, numInvocation ), std::shared_ptr<Mesh>(quadMesh), quadMaterial );
}


//--------------------------------------------------------------------------------------------------------------
MeshRenderer* TheRenderer::CreateCubeRenderer3D_PCUTB( const AABB3f& bounds, const std::string& shaderProgramName, const RenderState& renderState )
{
	static unsigned int numInvocation = 0;

	Mesh* cubeMesh = CreateAxisTintedCubeMesh3D_PCUTB( bounds );

	std::string name = "CubeTBN_" + shaderProgramName;
	Material* cubeMaterial = Material::CreateOrGetMaterial( name, &renderState, &Vertex3D_PCUTB::DEFINITION, shaderProgramName.c_str() );

	return MeshRenderer::CreateOrGetMeshRenderer( Stringf( "%s%d", name, numInvocation ), std::shared_ptr<Mesh>(cubeMesh), cubeMaterial );
}


//--------------------------------------------------------------------------------------------------------------
MeshRenderer* TheRenderer::CreateSphereRenderer3D_PCUTB( const Vector3f& spherePos, float radiusSizeRho, float numCirclesVertical, float numCirclesHorizontal, const Rgba& tint, const std::string& shaderProgramName, const RenderState& renderState )
{
	static unsigned int numInvocation = 0;

	Mesh* sphereMesh = CreateSphereMesh3D_PCUTB( spherePos, radiusSizeRho, numCirclesVertical, numCirclesHorizontal, tint );

	std::string name = "SphereTBN_" + shaderProgramName;
	Material* sphereMaterial = Material::CreateOrGetMaterial( name, &renderState, &Vertex3D_PCUTB::DEFINITION, shaderProgramName.c_str() );

	return MeshRenderer::CreateOrGetMeshRenderer( Stringf( "%s%d", name, numInvocation ), std::shared_ptr<Mesh>(sphereMesh), sphereMaterial );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::UpdateAndRenderPostProcess()
{
	//Only need to update the one that's actually being shown now, not loop entire container of MeshRenderers for post-process.
	MeshRenderer* mr = FramebufferEffect::GetFboEffectRenderer( s_currentFboEffectIndex );
	mr->SetMatrix4x4( "uModel", false, &Matrix4x4f::IDENTITY );
	mr->SetMatrix4x4( "uView", false, &Matrix4x4f::IDENTITY );
		//Use identity for view matrix to get fullscreen. To show FBO quad in-world somewhere, pass in current updated MVP or the view of a projector.
		//Have one cam in scene render to the texture, and another that projects it on things. Or a drone moving the view around, while you stand still.
	mr->SetMatrix4x4( "uProj", false, &Matrix4x4f::IDENTITY );

	mr->Render();
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::UpdateShaderTimersAndPositions( float deltaSeconds, const Camera3D* activeCam )
{
	//For time effect.
	static float wrappedDeltaSeconds = 0.f;
	static float unwrappedDeltaSeconds = 0.f;
	if ( !s_pauseTimerUniform )
	{
		wrappedDeltaSeconds += deltaSeconds;
		if ( wrappedDeltaSeconds > s_wrappedTimerDurationInSeconds )
			wrappedDeltaSeconds = 0.f;

		unwrappedDeltaSeconds += deltaSeconds;
	}

	Vector3f camPos = activeCam->m_worldPosition;
	for ( auto& sp : *ShaderProgram::GetRegistry() )
	{
		if ( !s_pauseTimerUniform )
		{
			sp.second->SetFloat( "uWrappingTimer", &wrappedDeltaSeconds );
			sp.second->SetFloat( "uUnwrappedTimer", &unwrappedDeltaSeconds );
		}
		sp.second->SetVector3( "uCameraPosition", &camPos );
	}
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::UpdateLights( float deltaSeconds, const Camera3D* activeCam )
{
	if ( s_isPilotingLight )
	{
		Light* pilotedLight = Light::GetLight( s_currentPilotedLightID );
		Vector3f camForwardXY = activeCam->GetForwardXY();
		Vector3f camLeftXY = activeCam->GetLeftXY();

		float speed = ( g_theInput->IsKeyDown( VK_SHIFT ) ) ? 8.f : 1.f;
		float deltaMove = ( speed * deltaSeconds ); //speed is then in units per second!

		if ( g_theInput->IsKeyDown( KEY_TO_MOVE_PILOTED_OBJECT_FORWARD ) )
			pilotedLight->Translate( camForwardXY * deltaMove );

		if ( g_theInput->IsKeyDown( KEY_TO_MOVE_PILOTED_OBJECT_BACKWARD ) )
			pilotedLight->Translate( -camForwardXY * deltaMove );

		if ( g_theInput->IsKeyDown( KEY_TO_MOVE_PILOTED_OBJECT_LEFT ) )
			pilotedLight->Translate( camLeftXY * deltaMove );

		if ( g_theInput->IsKeyDown( KEY_TO_MOVE_PILOTED_OBJECT_RIGHT ) )
			pilotedLight->Translate( -camLeftXY * deltaMove );

		if ( g_theInput->IsKeyDown( KEY_TO_MOVE_PILOTED_OBJECT_UP ) )
			pilotedLight->Translate( Vector3f( 0.f, 0.f, deltaMove ) ); //Scaled by z-axis, so *1.0f.

		if ( g_theInput->IsKeyDown( KEY_TO_MOVE_PILOTED_OBJECT_DOWN ) )
			pilotedLight->Translate( Vector3f( 0.f, 0.f, -deltaMove ) ); //Scaled by z-axis, so *1.0f.
	}

	//Parallel array setup.
	static Vector3f uLightPositions[ LIGHTS_IN_ENGINE_MAX ];
	static Vector3f uLightDirections[ LIGHTS_IN_ENGINE_MAX ];
	static Rgba uLightColors[ LIGHTS_IN_ENGINE_MAX ];

	static int uLightIsDirectionalFlags[ LIGHTS_IN_ENGINE_MAX ];

	static float uLightMinDistances[ LIGHTS_IN_ENGINE_MAX ];
	static float uLightMaxDistances[ LIGHTS_IN_ENGINE_MAX ];
	static float uLightPowersAtMinDists[ LIGHTS_IN_ENGINE_MAX ];
	static float uLightPowersAtMaxDists[ LIGHTS_IN_ENGINE_MAX ];
	static float uLightInnerAngleCosines[ LIGHTS_IN_ENGINE_MAX ];
	static float uLightOuterAngleCosines[ LIGHTS_IN_ENGINE_MAX ];
	static float uLightPowersInsideInnerAngles[ LIGHTS_IN_ENGINE_MAX ];
	static float uLightPowersOutsideOuterAngles[ LIGHTS_IN_ENGINE_MAX ];

	//Zero power by default.
	for ( unsigned int lightIndex = 0; lightIndex < LIGHTS_IN_ENGINE_MAX; lightIndex++ )
	{
		uLightPowersAtMinDists[ lightIndex ] = 0.f;
		uLightPowersAtMaxDists[ lightIndex ] = 0.f;
	}

	Light::GetLightPositions( uLightPositions );
	Light::GetLightDirections( uLightDirections );
	Light::GetLightColors( uLightColors );

	Light::GetLightIsDirectionalFlags( uLightIsDirectionalFlags );

	Light::GetLightMinDistances( uLightMinDistances );
	Light::GetLightMaxDistances( uLightMaxDistances );
	Light::GetLightPowersAtMinDists( uLightPowersAtMinDists );
	Light::GetLightPowersAtMaxDists( uLightPowersAtMaxDists );
	Light::GetLightInnerAngleCosines( uLightInnerAngleCosines );
	Light::GetLightOuterAngleCosines( uLightOuterAngleCosines );
	Light::GetLightPowersInsideInnerAngles( uLightPowersInsideInnerAngles );
	Light::GetLightPowersOutsideOuterAngles( uLightPowersOutsideOuterAngles );

	//Set uniforms for any multi-light shaders. Right now we know only BlinnPhongTBN shader uses them.
	for ( const std::pair< std::string, ShaderProgram* >& registryShader : *ShaderProgram::GetRegistry() )
	{
		ShaderProgram& sp = *registryShader.second;

		sp.SetVector3( "uLightPositions[0]", uLightPositions, LIGHTS_IN_ENGINE_MAX ); //Should it be *uLightPositions, since the function uses &val?
		sp.SetVector3( "uLightDirections[0]", uLightDirections, LIGHTS_IN_ENGINE_MAX );
		sp.SetColor( "uLightColors[0]", uLightColors, LIGHTS_IN_ENGINE_MAX );

		sp.SetFloat( "uLightMinDistances[0]", uLightMinDistances, LIGHTS_IN_ENGINE_MAX );
		sp.SetFloat( "uLightMaxDistances[0]", uLightMaxDistances, LIGHTS_IN_ENGINE_MAX );

		sp.SetFloat( "uLightPowersAtMinDists[0]", uLightPowersAtMinDists, LIGHTS_IN_ENGINE_MAX );
		sp.SetFloat( "uLightPowersAtMaxDists[0]", uLightPowersAtMaxDists, LIGHTS_IN_ENGINE_MAX );

		sp.SetFloat( "uLightInnerAngleCosines[0]", uLightInnerAngleCosines, LIGHTS_IN_ENGINE_MAX );
		sp.SetFloat( "uLightOuterAngleCosines[0]", uLightOuterAngleCosines, LIGHTS_IN_ENGINE_MAX );

		sp.SetFloat( "uLightPowersInsideInnerAngles[0]", uLightPowersInsideInnerAngles, LIGHTS_IN_ENGINE_MAX );
		sp.SetFloat( "uLightPowersOutsideOuterAngles[0]", uLightPowersOutsideOuterAngles, LIGHTS_IN_ENGINE_MAX );

		sp.SetInt( "uLightIsDirectionalFlags[0]", uLightIsDirectionalFlags, LIGHTS_IN_ENGINE_MAX );
	}
}


//--------------------------------------------------------------------------------------------------------------
// Called prior to TheRenderer's render loop to set 3D (so perspective proj, non-identity view) transforms.
void TheRenderer::UpdateSceneMVP( const Camera3D* activeCam )
{
	Matrix4x4f view = Matrix4x4f( COLUMN_MAJOR ); //From camera.
	const Vector3f& camPos = activeCam->m_worldPosition;
	static double aspect = m_screenWidth / m_screenHeight; //Probably sync this up at program start.

	//Detour Change of Basis Theorem: convert to Forseth's basis, apply his rotation matrix transform, convert back.
	view.ClearToRotationMatrix_MyBasis( //takes this order: yaw, pitch, roll.
										-activeCam->m_orientation.m_yawDegrees,		//static test shows this was around world up j-vector as desired.
										-activeCam->m_orientation.m_pitchDegrees,	//static test shows this was around world right i-vector as desired.
										-activeCam->m_orientation.m_rollDegrees,		//so this is around k.
										view.GetOrdering() ); //Note these and all transforms are the same order as TheGame::ApplyCamTransform, but there it's pushed on a stack, hence "reversed".
	view.SetTranslation( camPos, view.GetOrdering() );

	//Rather than using the view relative to the world, we want the world relative to the camera.	
	view.GetInverseAssumingOrthonormality( view );

	//Note model transform isn't currently in use.
	for ( const ShaderProgramRegistryPair& shaderPair : *ShaderProgram::GetRegistry() )
	{
		shaderPair.second->SetMatrix4x4( "uModel", false, &Matrix4x4f::IDENTITY );
		shaderPair.second->SetMatrix4x4( "uView", false, &view );
	}
	//NOTE: must be overwritten per frame for other view transform needs, e.g. in SetupView2D for screen-space rendering.
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::Update( float deltaSeconds, const Camera3D* activeCam )
{
	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_ORIGIN_AXES ) )
	{
		Command cmd( "ToggleOriginAxes 1" );
		ToggleAxes( cmd );
	}

	UpdateShaderTimersAndPositions( deltaSeconds, activeCam );
	UpdateSceneMVP( activeCam );
	UpdateLights( deltaSeconds, activeCam );
}


//--------------------------------------------------------------------------------------------------------------
Mesh* TheRenderer::CreateSurfacePatchMesh3D( Expression graphingInput, const VertexDefinition* vertexDefinition,
												   float startX, float endX, unsigned int xSectionCount,
												   float startY, float endY, unsigned int ySectionCount,
												   bool storeMeshInSingleton /*= false*/,
												   const Rgba& tint /*= Rgba::WHITE*/ )
{
	MeshBuilder builder;

	builder.SetColor( tint );

	builder.BuildSurfacePatch( graphingInput, nullptr, startX, endX, xSectionCount, startY, endY, ySectionCount );

	Mesh* mesh = new Mesh( *vertexDefinition );
	builder.CopyToMesh( mesh );

	if ( storeMeshInSingleton )
	{
		if ( g_lastLoadedMeshBuilder != nullptr )
		{
			delete g_lastLoadedMeshBuilder;
			g_lastLoadedMeshBuilder = nullptr;
		}
		g_lastLoadedMeshBuilder = new MeshBuilder( builder );
		g_lastLoadedMeshBuilder->SetVertexDataMaskFromVertexDefinition( vertexDefinition );
	}

	return mesh;
}


//--------------------------------------------------------------------------------------------------------------
Mesh* TheRenderer::CreateTriangleMesh3D( const VertexDefinition* vertexDefinition, const Vector3f& topLeft, const Vector3f& bottomLeft, const Vector3f& bottomRight, const Rgba& tint /*= Rgba::WHITE*/ )
{
	MeshBuilder builder;
	builder.SetColor( tint );
	builder.BuildTriangle( topLeft, bottomLeft, bottomRight );

	Mesh* mesh = new Mesh( *vertexDefinition );
	builder.CopyToMesh( mesh );

	return mesh;
}


//--------------------------------------------------------------------------------------------------------------
static Material* MapToEngineMaterial( const std::string& materialNameID, const VertexDefinition* vdefn )
{
	TODO( "STD::MAP INSTEAD" );
	static unsigned int numInvocation = 0;

//	if ( materialNameID == "Tutorial Box" )
//		return Material::CreateOrGetMaterial( "FboEffect_PostProcessObama", &RenderState::DEFAULT, vdefn, "FboEffect_PostProcessObama" );


	//-----------------------------------------------------------------------------
	//Unity-chan
	
	if ( materialNameID == "body" )
		return Material::CreateOrGetMaterial( "UnityChan_Body", &RenderState::DEFAULT, vdefn, "UnityChan_Body" );
	if ( materialNameID == "eyebase" )
		return Material::CreateOrGetMaterial( "UnityChan_Eyebase", &RenderState::DEFAULT, vdefn, "UnityChan_Eyebase" );
	if ( materialNameID == "eye_L1" )
		return Material::CreateOrGetMaterial( "UnityChan_EyeIrisL", &RenderState::DEFAULT, vdefn, "UnityChan_EyeIrisL" );
	if ( materialNameID == "eye_R1" )
		return Material::CreateOrGetMaterial( "UnityChan_EyeIrisR", &RenderState::DEFAULT, vdefn, "UnityChan_EyeIrisR" );
	if ( materialNameID == "eyeline" )
		return Material::CreateOrGetMaterial( "UnityChan_Eyeline", &RenderState::DEFAULT, vdefn, "UnityChan_Eyeline" );
	if ( materialNameID == "face" )
		return Material::CreateOrGetMaterial( "UnityChan_Face", &RenderState::DEFAULT, vdefn, "UnityChan_Face" );
	if ( materialNameID == "hair" )
		return Material::CreateOrGetMaterial( "UnityChan_Hair", &RenderState::DEFAULT, vdefn, "UnityChan_Hair" );
	if ( materialNameID == "skin1" )
		return Material::CreateOrGetMaterial( "UnityChan_Skin", &RenderState::DEFAULT, vdefn, "UnityChan_Skin" );
	if ( materialNameID == "mat_cheek" )
		return Material::CreateOrGetMaterial( "UnityChan_Cheek", &RenderState::DEFAULT, vdefn, "UnityChan_Cheek" );

	//-----------------------------------------------------------------------------
	else
	{
		DebuggerPrintf( "Found material ID without a suitable engine-side match: %s. Using default.", materialNameID.c_str() );
		return Material::CreateOrGetMaterial( Stringf( "ModelMappedMaterial%d", ++numInvocation ), &RenderState::DEFAULT, vdefn,
											   ShaderProgram::GetDefaultShaderNameForVertexDefinition( vdefn ) );
	}
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::AddModel( MeshBuilder* modelMesh )
{
	//Copies a MeshBuilder's current state into a static mesh, makes a material if none provided, adds to MeshRenderer.

	static unsigned int numInvocation = 0;
	const VertexDefinition* modelVertexDefinition = modelMesh->GetVertexDefinitionFromVertexDataMask();

	Mesh* mesh = new Mesh( *modelVertexDefinition );
	modelMesh->CopyToMesh( mesh ); //Put under Vertexes.hpp/cpp.

	Material* modelMaterial = nullptr;
	std::string materialNameID = modelMesh->GetMaterialID();
	if ( materialNameID == "" )
		modelMaterial = Material::CreateOrGetMaterial( Stringf( "ModelMaterial%d", numInvocation ), &RenderState::DEFAULT, modelVertexDefinition, 
													   ShaderProgram::GetDefaultShaderNameForVertexDefinition( modelVertexDefinition ) );
	else 
		modelMaterial = MapToEngineMaterial( materialNameID, modelVertexDefinition );
	MeshRenderer::CreateOrGetMeshRenderer( Stringf( "ModelRenderer%d", numInvocation ), std::shared_ptr<Mesh>( mesh ), modelMaterial );

	++numInvocation;
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::VisualizeCurrentSkeletons()
{
	for ( Skeleton* sk : m_skeletonVisualizations )
		if ( sk != nullptr ) 
			sk->VisualizeSkeleton();
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DeleteSkeletons()
{
	for ( Skeleton* sk : m_skeletonVisualizations )
		if ( sk != nullptr )
			delete sk;
	m_skeletonVisualizations.clear();
	g_lastLoadedSkeleton = nullptr; //Just in case.
	g_animationPaused = true;
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DeleteAnimations()
{
	for ( AnimationSequence* as : m_animationSequences )
		if ( as != nullptr )
			delete as;
	m_animationSequences.clear();
	g_animationPaused = true;
}


//--------------------------------------------------------------------------------------------------------------
static bool g_goingBackwardsNow = false;
void TheRenderer::ApplyAnimationToSkeleton( int numAnimation, int numSkeleton )
{
	if ( g_animationMode == ANIMATION_MODE_PINGPONG && g_animationTimer < 0.f )
	{
		g_animationTimer = 0.f;
		g_goingBackwardsNow = false;
	}

	m_animationSequences[ numAnimation ]->ApplyMotionToSkeleton( m_skeletonVisualizations[ numSkeleton ], g_animationTimer );
	const int MAX_BONES = 200; //In total on skeleton.
	Matrix4x4f currentBoneTransforms[ MAX_BONES ];
	m_skeletonVisualizations.front()->GetBoneMatrices( currentBoneTransforms, MAX_BONES );
	for ( auto& sp : *ShaderProgram::GetRegistry() )
		sp.second->SetMatrix4x4( "uBoneMatrices[0]", true, currentBoneTransforms, MAX_BONES );
//	delete[] currentBoneTransforms;


	if ( g_animationMode != ANIMATION_MODE_PINGPONG || !g_goingBackwardsNow )
		g_animationTimer += m_animationSequences[ numAnimation ]->m_keyframeLengthSeconds;
	else if ( g_animationMode == ANIMATION_MODE_PINGPONG && g_goingBackwardsNow )
		g_animationTimer -= m_animationSequences[ numAnimation ]->m_keyframeLengthSeconds;

	if ( g_animationTimer > m_animationSequences[ numAnimation ]->m_animationLengthSeconds )
	{
		if ( g_animationMode == ANIMATION_MODE_LOOP )
			g_animationTimer = 0.f;
		else if ( g_animationMode == ANIMATION_MODE_CLAMP )
			g_animationTimer = m_animationSequences[ numAnimation ]->m_animationLengthSeconds;
		else if ( g_animationMode == ANIMATION_MODE_PINGPONG )
			g_goingBackwardsNow = true;
	}
}

//--------------------------------------------------------------------------------------------------------------
void TheRenderer::PostRenderStep()
{
#ifdef RENDER_2D_ON_WORLD_QUAD
	if ( g_theRenderer->IsShowingFBOs() )
		g_theRenderer->BindFBO( m_lastNonDefaultFBO );
	else
		g_theRenderer->BindFBO( nullptr );

	TODO( "Why is this necessary, but other FBOs don't have to do it? Are the others being excluded from loop over shader registry somehow?" );
	m_render2DOnWorldQuadRenderer->SetTexture( "uTexDiffuse", m_render2DOnWorldQuadFBO->GetColorTextureID( 0 ) );
	m_render2DOnWorldQuadRenderer->Render(); //Now render the UI quad in world.
#endif

	if ( !g_animationPaused )
		ApplyAnimationToSkeleton( g_lastAnimationSelected, g_lastSkeletonSelected );

	g_theRenderer->VisualizeCurrentSkeletons();

	if ( TheRenderer::IsShowingFBOs() )
	{
		g_theRenderer->BindFBO( nullptr );	
		UpdateAndRenderPostProcess();
	}

#ifdef PLATFORM_RIFT_CV1
	int eye = m_riftContext->currentEye;
	m_riftContext->eyeRenderTexture[ eye ]->SetRenderSurface( m_riftContext->eyeDepthBuffer[ eye ] );
#endif

}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::PreRenderStep()
{
	g_theRenderer->ClearScreenToColor( Rgba::MAGENTA ); //BG color of "no-FBO no-3D no-2D" world behind game rendering.
	g_theRenderer->ClearScreenDepthBuffer();
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::AddFullscreenFboEffects( const char** fboEffectShaderProgramNames, unsigned int numNames )
{
	if ( numNames == 0 )
		return;

	for ( unsigned int fboEffectIndex = 0; fboEffectIndex < numNames; fboEffectIndex++ )
		AddFullscreenFboEffect( fboEffectShaderProgramNames[ fboEffectIndex ] );
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::AddFullscreenFboEffect( const char* fboEffectShaderProgramName )
{
	Material* quadMaterial;
	quadMaterial = Material::CreateOrGetMaterial( Stringf( "FboEffect_%s", fboEffectShaderProgramName ), &TheRenderer::DEFAULT_FBO_RENDER_STATE, &Vertex3D_PCUTB::DEFINITION, fboEffectShaderProgramName );
	quadMaterial->SetMatrix4x4( "uModel", false, &Matrix4x4f::IDENTITY );
	quadMaterial->SetMatrix4x4( "uView", false, &Matrix4x4f::IDENTITY ); //Identity == fullscreen. To show FBO quad in-world somewhere, pass in current updated MVP or the view of a projector.
		//e.g. Have one cam in scene render to the texture, and another that projects it on things. Or a drone moving the view around, while you stand still.
	quadMaterial->SetMatrix4x4( "uProj", false, &Matrix4x4f::IDENTITY );

	//Currently a outputting 1 color render target, 1 depth-and-stencil render target.
	quadMaterial->SetTexture( "uTexDiffuse", m_lastNonDefaultFBO->GetColorTextureID( 0 ) );
	quadMaterial->SetSampler( "uTexDiffuse", TheRenderer::DEFAULT_SAMPLER_ID );
	quadMaterial->SetTexture( "uTexDepth", m_lastNonDefaultFBO->GetDepthStencilTextureID() );
	quadMaterial->SetSampler( "uTexDepth", TheRenderer::DEFAULT_SAMPLER_ID );

	FramebufferEffect::CreateAndAddFboEffectRenderer( quadMaterial->GetName(), m_defaultFboQuad, quadMaterial );
}


//--------------------------------------------------------------------------------------------------------------
#ifdef PLATFORM_RIFT_CV1
void TheRenderer::RiftShutdown()
{
	if (m_riftContext->mirrorFBO)
		glDeleteFramebuffers(1, &m_riftContext->mirrorFBO);

	if (m_riftContext->mirrorTexture)
		ovr_DestroyMirrorTexture(m_riftContext->session, m_riftContext->mirrorTexture);

	for (int eye = 0; eye < 2; ++eye)
	{
		delete m_riftContext->eyeRenderTexture[eye];
		delete m_riftContext->eyeDepthBuffer[eye];
	}
	ovr_Destroy(m_riftContext->session);

	ovr_Shutdown(); //Has been proven to work even when not at the very end of the program.

	if (m_riftContext != nullptr)
	{
		delete m_riftContext;
		m_riftContext = nullptr;
	}
}
#endif


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::Shutdown()
{
#ifdef PLATFORM_RIFT_CV1
	RiftShutdown();
#endif

	DeleteFonts();
	DeleteTextures();
	
	TODO( "Delete remaining OpenGL Resources, including textures, samplers, meshes, materials, etc. but GPU-side." );
	for ( const MeshRendererRegistryPair& mr : *MeshRenderer::GetMeshRendererRegistry() )
	{
		MeshRenderer* mrPtr = mr.second;
		delete mrPtr;
		mrPtr = nullptr;
	}
	for ( const FboEffectRegistryPair& fboEffectPair : *FramebufferEffect::GetFboEffectRendererRegistry() )
	{
		FramebufferEffect* fboEffect = fboEffectPair.second;
		MeshRenderer* mrPtr = fboEffect->m_fboEffectRenderer;
		delete fboEffect;
		fboEffect = nullptr;
		delete mrPtr;
		mrPtr = nullptr;
	}
	FIXME( "Why does delete m_lastNonDefaultFBO crash below?" );
	//	delete m_lastNonDefaultFBO; 
	//	m_lastNonDefaultFBO = nullptr;

	DeleteSkeletons();
	DeleteAnimations();
	Light::DeleteLights();
	Material::DeleteMaterials(); //s_defaultMaterial2D and 3D are cleaned by this.
	ShaderProgram::DeleteShaderPrograms();
	Shader::DeleteShaders();

	delete s_defaultRenderer;
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DeleteTextures()
{
	for ( auto& tex : Texture::GetRegistry() ) 
		delete tex.second;

	delete m_defaultSampler;
	m_defaultSampler = nullptr;
}


//--------------------------------------------------------------------------------------------------------------
static void SetupPerspectiveProjection( const Camera3D* activeCamera )
{
#ifdef PLATFORM_RIFT_CV1
	//Override the normal 3D uView and uProj set by UpdateSceneMVP (prior to the engine calling render which calls this via SetupView3D).
	int eye = g_theRenderer->GetRiftContext()->currentEye;
	Matrix4x4f view = g_theRenderer->CalcRiftViewMatrixMyBasis( eye, activeCamera );
	Matrix4x4f proj = g_theRenderer->CalcRiftPerspProjMatrixMyBasis( eye );

	for ( auto& sp : *ShaderProgram::GetRegistry() )
	{
		sp.second->SetMatrix4x4( "uModel", false, &Matrix4x4f::IDENTITY );
		sp.second->SetMatrix4x4( "uView", false, &view );
		sp.second->SetMatrix4x4( "uProj", false, &proj );
	}
#else 
	UNREFERENCED( activeCamera );

	float aspect = (float)( g_theRenderer->GetScreenWidth() / g_theRenderer->GetScreenHeight() ); //WARNING: speed-critical / per-frame division.
	g_theRenderer->SetPerspective( ENGINE_PERSPECTIVE_FOV_Y_DEGREES, aspect, ENGINE_Z_NEAR, ENGINE_Z_FAR );
#endif
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::SetupView3D( const Camera3D* activeCamera )
{
	g_theRenderer->ClearScreenToColor( Rgba::BLACK ); //BG color of FBOs-off world.
	g_theRenderer->ClearScreenDepthBuffer();

	if ( g_theRenderer->IsShowingFBOs() )
	{
		g_theRenderer->BindFBO( g_theRenderer->GetCurrentFBO() );
		g_theRenderer->ClearScreenToColor( Rgba::DARK_GRAY ); //BG color of FBOs-on world.
		g_theRenderer->ClearScreenDepthBuffer();
	}

	SetupPerspectiveProjection( activeCamera );

	//uView is set for all shader programs in UpdateSceneMVP.
}


//--------------------------------------------------------------------------------------------------------------
Vector2f TheRenderer::GetScreenCenter() const
{
	Vector2f screenCenter;

	screenCenter.x = static_cast<float>( m_screenWidth / 2.0 );
	screenCenter.y = static_cast<float>( m_screenHeight / 2.0 );

	return screenCenter;
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::SetupView2D( const Camera2D* activeCamera )
{
#ifdef RENDER_2D_ON_WORLD_QUAD
	g_theRenderer->BindFBO( m_render2DOnWorldQuadFBO, false );
	g_theRenderer->ClearScreenToColor( Rgba::CYAN ); //BG color of FBOs-on world.
	g_theRenderer->ClearScreenDepthBuffer();
#endif

//#ifdef PLATFORM_RIFT_CV1 //Overwrite by adding in offsets based on VR HMD.
//	int eye = g_theRenderer->GetRiftContext()->currentEye;
//	Matrix4x4f view = g_theRenderer->CalcRiftViewMatrixMyBasis( eye, activeCamera );
//	Matrix4x4f ortho = g_theRenderer->CalcRiftOrthoProjMatrixMyBasis( eye );
//	s_defaultMaterial2D->SetMatrix4x4( "uView", false, &view ); //I get the feeling it needs a different math somehow?
//	s_defaultMaterial2D->SetMatrix4x4( "uProj", false, &ortho );
//#else
	UNREFERENCED( activeCamera );

	s_defaultMaterial2D->SetMatrix4x4( "uView", false, &Matrix4x4f::IDENTITY );
		//Because unfortunately the loop in UpdateSceneMVP overwrites this value each frame.

	g_theRenderer->SetOrtho( Vector2f::ZERO, Vector2f( (float)GetScreenWidth(), (float)GetScreenHeight() ) );
 //#endif
}


//--------------------------------------------------------------------------------------------------------------
void TheRenderer::DeleteFonts()
{
	for ( auto& font : BitmapFont::GetFontRegistry() ) 
		delete font.second; //Handles m_defaultProportionalFont.
	BitmapFont::ClearFontRegistry();
	m_defaultProportionalFont = nullptr;

	for ( auto& font : FixedBitmapFont::GetFontRegistry() ) 
		delete font.second; //Handles m_defaultMonospaceFont.
	FixedBitmapFont::ClearFontRegistry();
	m_defaultMonospaceFont = nullptr;
}
