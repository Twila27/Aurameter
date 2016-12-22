#pragma once

#include "Engine/Tools/Profiling/ProfileSection.hpp"


//-----------------------------------------------------------------------------
#define PROFILE_SCOPE(tag) ProfileScopedSection _s_##__LINE__##(tag)
/* Example Usage
	void MyFuncToProfile()
	[
		PROFILE_SCOPE( "MyFuncToProfile" ); //Name to show under profiled functions.
	]
*/


//-----------------------------------------------------------------------------
class ProfileScopedSection
{
	ProfileSection m_section;
	ProfileScopedSection();
	~ProfileScopedSection();
};
