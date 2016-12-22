#pragma once

#include "Engine/Tools/Profiling/ProfileSection.hpp"


//-----------------------------------------------------------------------------
#if PROFILER_MODE >= PROFILER_LOG_SECTIONS_ONLY
	#define PROFILE_LOG_SECTION(tag) ProfileLogSection _s_##__LINE__##(tag)
#else
	#define PROFILE_LOG_SECTION(tag) 
#endif
//NOTE: the "_s_" is just giving it a unique identifier.
//RECALL: a single # converts a value to a string, a double ## concatenates.

/* Example Usage
	void MyFuncToProfile()
	[
		PROFILE_PRINT_SCOPE("Foo"); //If this is line 12, becomes ProfilePrintScoped _s_12("Foo"); to declare a local var that will be destroyed at function end.
	
		if ( something )
		[
			PROFILE_PRINT_SCOPE("Foo"); //Will be unique from above macro because of its unique line #.
		]
	]
*/

//"Since there is logging and flushing going on, having these in your code will slow down your code, and preferably should never be checked in."
//"But it’s a good tool to have when debugging. Put one in a function, and then put a breakpoint after the function is called, and look at the log."


//-----------------------------------------------------------------------------
class ProfileLogSection
{
public:
	ProfileLogSection( const char* id );
	~ProfileLogSection();


private:
	ProfileSection m_section;
	const char* m_id;
};
