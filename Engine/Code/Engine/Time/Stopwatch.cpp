#include "Engine/Time/Stopwatch.hpp"
#include "Engine/Core/TheEventSystem.hpp"
#include "Engine/Core/EngineEvent.hpp"


void Stopwatch::Update( float deltaSeconds )
{
	if ( m_isPaused )
		return;

	m_currentTimeSeconds += deltaSeconds;

	if ( m_currentTimeSeconds > m_endTimeSeconds )
	{
		if ( m_endingEventID != nullptr )
			TheEventSystem::Instance()->TriggerEvent( m_endingEventID );

		Stop();
	}
}
