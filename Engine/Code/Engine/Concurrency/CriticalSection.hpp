#pragma once

#include <mutex>


//Wraps the implementation of a critical section in a class.
class CriticalSection 
{
private:
	std::mutex m_mutex;


public:
	void Lock() { m_mutex.lock(); }
	void Unlock() { m_mutex.unlock(); }
	bool TryLock() { m_mutex.try_lock(); }

	CriticalSection() {}
	CriticalSection( const CriticalSection& copy ) = delete;
};


/* Windows Equivalent API - generally "faster", but not portable.
	CRITICAL_SECTION cs;
	InitializeCriticalSectionAndSpinCount( &cs, 8 );
	EnterCriticalSection( &cs );
	LeaveCriticalSection( &cs );
	TryEnterCriticalSection( &cs );
*/