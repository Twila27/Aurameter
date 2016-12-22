#pragma once

#include <thread>

typedef void(ThreadFunctionPtr)(void*);

class Thread
{
private:
	std::thread m_handle;

public:
	Thread( ThreadFunctionPtr* entryFunctionPtr, void* args = nullptr ) { m_handle = std::thread( entryFunctionPtr, args ); } //Only 0-arity for now. Always hits heap. :(
	void ThreadDetach() { m_handle.detach(); } //Allowed to clean itself up immediately on completion, i.e. no dependencies exist.
	void ThreadJoin() { m_handle.join(); } //Stalls until finished before continuing main thread, i.e. should dependencies exist.
	static void ThreadSleep( std::chrono::milliseconds ms ) { std::this_thread::sleep_for( ms ); } //Do not call with <= 10ms, practically no effect.
	static void ThreadYield() { std::this_thread::yield(); } //Says "I have nothing to do, let other threads have their chance to work."
};
