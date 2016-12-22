#pragma once


#include "Engine/EngineCommon.hpp"


//-----------------------------------------------------------------------------
class Stopwatch
{
public:
	Stopwatch( float endTimeSeconds, EngineEventID endingEventID = nullptr )
		: m_currentTimeSeconds( 0.f )
		, m_endTimeSeconds( endTimeSeconds )
		, m_endingEventID( endingEventID )
	{
	}

	inline void Start() { Unpause(); }
	inline void Stop() { Reset(); Pause(); }

	inline void Reset() { m_currentTimeSeconds = 0.f; }
	inline void Unpause() { m_isPaused = false; }
	inline void Pause() { m_isPaused = true; }
	inline bool IsPaused() const { return m_isPaused; }

	inline void SetEndTimeSeconds( float newTime ) { m_endTimeSeconds = newTime; }
	void Update( float deltaSeconds );


private:
	bool m_isPaused;
	float m_endTimeSeconds;
	float m_currentTimeSeconds;
	EngineEventID m_endingEventID;
};