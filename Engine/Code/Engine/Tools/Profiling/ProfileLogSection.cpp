#include "Engine/Tools/Profiling/ProfileLogSection.hpp"
#include "Engine/Core/Logger.hpp"


//--------------------------------------------------------------------------------------------------------------
ProfileLogSection::ProfileLogSection( const char* id )
{
	m_id = id;
	m_section.Start();
}


//--------------------------------------------------------------------------------------------------------------
ProfileLogSection::~ProfileLogSection()
{
	m_section.End();
	Logger::PrintfWithTag( "Profiler", "%s took %.8f seconds.", m_id, m_section.GetElapsedSeconds() );
	Logger::Flush();
	//Downside of the print allocating a string on the heap and slowdown from flush.
}
