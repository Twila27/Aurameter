#include "Engine/Tools/Profiling/ProfileScopedSection.hpp"


//--------------------------------------------------------------------------------------------------------------
ProfileScopedSection::ProfileScopedSection()
{
	m_section.Start();
}


//--------------------------------------------------------------------------------------------------------------
ProfileScopedSection::~ProfileScopedSection()
{
	m_section.End();
}
