#pragma once

#include "Engine/Concurrency/CriticalSection.hpp"
#include "Engine/Memory/UntrackedAllocator.hpp"
#include <queue>


template < typename T >
class ThreadSafeQueue : protected std::queue<T, std::deque<T, UntrackedAllocator<T> > > //Prevent access but through this interface.
{
private:
	CriticalSection criticalSection;

public:
	ThreadSafeQueue<T>() : std::queue<T, std::deque<T, UntrackedAllocator<T> > >()
	{
	}
	void Enqueue( T const& value )
	{
		criticalSection.Lock();
		{
			this->push( value );
		}
		criticalSection.Unlock();
	}
	bool Dequeue( T* out )
	{
		bool result = false;

		criticalSection.Lock();
		{
			if ( !this->empty() )
			{
				*out = this->front();
				this->pop();
				result = true;
			}
		}
		criticalSection.Unlock();

		return result;
	}
};