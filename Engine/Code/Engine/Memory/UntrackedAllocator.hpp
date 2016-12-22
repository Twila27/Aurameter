#pragma once
//Attribution for Class: SD5 Prof. Chris Forseth

#include <vector>
#include <limits.h>
#undef max

//--------------------------------------------------------------------------------------------------------------
template <typename T>
class UntrackedAllocator
{
public: //Typedefs.
	typedef T value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

public: //Convert allocator<T> to allocator<U>.
	template<typename U>
	struct rebind 
	{ 
		typedef UntrackedAllocator<U> other; 
	};

public:
	inline explicit UntrackedAllocator() {}
	inline ~UntrackedAllocator() {}
	inline explicit UntrackedAllocator( UntrackedAllocator const& ) {}
	template<typename U> inline explicit UntrackedAllocator( UntrackedAllocator<U> const& ) {}

	//Address.
	inline pointer address( reference r )
	{
		return &r;
	}
	inline const_pointer address( const_reference r )
	{
		return &r;
	}

	//Memory allocation.
	inline pointer allocate( size_type count, typename std::allocator<void>::const_pointer = 0 )
	{
		T* ptr = (T*)malloc( count * sizeof( T ) );
		return ptr;
	}
	inline void deallocate( pointer p, size_type count )
	{
		(void)( count ); //Unreferenced parameter.
		free( p );
	}

	//Size.
	inline size_type getMaxSize() const
	{
		return std::numeric_limits<size_type>::max() / sizeof( T );
	}

	//Ctor and dtor.
	inline void construct( pointer p, const T& t )
	{
		new( p ) T( t );
	}
	inline void destroy( pointer p )
	{
		p->~T();
		(void)( p ); //Unreferenced parameter warning thrown for some reason?
	}

	inline bool operator==( UntrackedAllocator const& a ) { return this == &a; }
	inline bool operator!=( UntrackedAllocator const& a ) { return !operator==(a); }
};
