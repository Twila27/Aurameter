#pragma once

#include "Engine/Concurrency/CriticalSection.hpp"
#include "Engine/Memory/UntrackedAllocator.hpp"
#include <vector>


template < typename T >
class ThreadSafeVector : protected std::vector<T, UntrackedAllocator<T> > //Prevent access but through this interface.
{
private:
	CriticalSection criticalSection;

public:
	ThreadSafeVector<T>() : std::vector<T, UntrackedAllocator<T> >()
	{
	}
	void Clear()
	{
		criticalSection.Lock();
		{
			std::vector<T, UntrackedAllocator<T> >::clear();
		}
		criticalSection.Unlock();
	}
	bool Find( const char*& target, typename std::vector< const char*, UntrackedAllocator<const char*> >::iterator* outPosition )
	{
		bool found = false;

		criticalSection.Lock();
		{
			unsigned int size = std::vector< const char*, UntrackedAllocator<const char*> >::size();
			for ( unsigned int elementIndex = 0; elementIndex < size; elementIndex++ )
			{
				if ( strcmp( this->at( elementIndex ), target ) == 0 )
				{
					found = true;
					if ( outPosition != nullptr )
						*outPosition = this->begin() + elementIndex;

					break;
				}
			}
		}
		criticalSection.Unlock();

		return found;
	}
	bool Find( const T& target, typename std::vector< T, UntrackedAllocator<T> >::iterator* outPosition )
	{
		bool found = false;

		criticalSection.Lock();
		{
			unsigned int size = std::vector< T, UntrackedAllocator<T> >::size();
			for ( unsigned int elementIndex = 0; elementIndex < size; elementIndex++ )
			{
				if ( this->at( elementIndex ) == target )
				{
					found = true;
					*outPosition = this->begin() + elementIndex;
					break;
				}
			}
		}
		criticalSection.Unlock();

		return found;
	}
	bool At( unsigned int index, T* out )
	{
		bool result = false;

		criticalSection.Lock();
		{
			if ( !this->empty() )
			{
				*out = std::vector< T, UntrackedAllocator<T> >::at( index );
				result = true;
			}
		}
		criticalSection.Unlock();

		return result;
	}
	void PushBack( T const& value )
	{
		criticalSection.Lock();
		{
			this->push_back( value );
		}
		criticalSection.Unlock();
	}
	bool Erase( typename std::vector< T, UntrackedAllocator<T> >::const_iterator position, 
				typename std::vector< T, UntrackedAllocator<T> >::iterator* out )
	{
		bool result = false;

		criticalSection.Lock();
		{
			if ( !this->empty() )
			{
				std::vector< T, UntrackedAllocator<T> >::iterator retval = std::vector< T, UntrackedAllocator<T> >::erase( position );
				if ( out != nullptr )
					*out = retval;

				result = true;
			}
		}
		criticalSection.Unlock();

		return result;
	}
	unsigned int Size()
	{
		unsigned int size;
		criticalSection.Lock();
		{
			size = std::vector< T, UntrackedAllocator<T> >::size();
		}
		criticalSection.Unlock();
		return size;
	}
};