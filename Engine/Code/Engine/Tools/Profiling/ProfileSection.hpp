#pragma once


//-----------------------------------------------------------------------------
typedef unsigned long long uint64_t;


//-----------------------------------------------------------------------------
class ProfileSection
{
public:
	void Start();
	void End();
	double GetElapsedSeconds() const;


private:
	uint64_t m_startPerfCount;
	uint64_t m_elapsedPerfCount; //AKA deltaTime.
};
