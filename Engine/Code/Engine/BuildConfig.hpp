#pragma once

//TODO( "BREAK INTO A SEPARATE #INCLUDED FILE FOR THE CONSTANTS, SO WE ONLY SHOW VALUES USERS SHOULD CHANGE." );

//DFS1 VR
//#define PLATFORM_RIFT_CV1
//#define RENDER_2D_ON_WORLD_QUAD

//SD5 A1 Memory Analytics
#define MEMORY_DETECTION_NONE			-1
#define MEMORY_DETECTION_BASIC			0
#define MEMORY_DETECTION_VERBOSE		1

#define MEMORY_DETECTION_MODE			MEMORY_DETECTION_NONE
//--


//SD5 A2 Logger
#if defined(_WIN32) || defined(_WIN64)
	#define PLATFORM_WINDOWS
#endif

#ifdef PLATFORM_WINDOWS
	#define LOGGER_OUTPUT_TO_VSDEBUGGER
#endif

#define LOG_FILE_PATH "debug.log"
#define LOGGER_DEBUG
//--

//SD5 A3 SpriteRenderer
#define SPRITE_DEGREES_CORRECTION_DEFAULT_TO_UP			0.f
#define SPRITE_DEGREES_CORRECTION_DEFAULT_TO_RIGHT		90.f
#define SPRITERENDERER_FORWARD_IS_UP					SPRITE_DEGREES_CORRECTION_DEFAULT_TO_UP
#define SPRITERENDERER_FORWARD_IS_RIGHT					SPRITE_DEGREES_CORRECTION_DEFAULT_TO_RIGHT

#define SPRITERENDERER_FORWARD_DIRECTION				SPRITERENDERER_FORWARD_IS_UP
//--


//SD5 A5 Profiler
#define PROFILER_NONE					-1
#define PROFILER_LOG_SECTIONS_ONLY		0
#define PROFILER_FULL_FRAME_SAMPLING	1
#define PROFILER_MODE					PROFILER_FULL_FRAME_SAMPLING
//--

/* Examples of Other Settings
	#ifdef __MSC_VER 
		#ifdef (_WIN32)
			...
		#endif
		#ifdef (_WIN64)
			...
		#endif
		#if !defined(_WIN32) && !defined(_WIN64)
			...
		#endif
	#endif

	#define CAN_LOAD_JPEG
	#define HAS_NETWORKING_SUPPORT, HAS_AUDIO_SUPPORT
	#define USES_OPENGL, USES_DIRECTX
*/