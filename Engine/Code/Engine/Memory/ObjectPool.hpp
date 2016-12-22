#pragma once


#include "Engine/Memory/PageAllocator.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/Concurrency/CriticalSection.hpp"
#pragma warning ( disable : 4127 ) //Constant conditional in ASSERT_OR_DIE below (after template instantiation).


//-----------------------------------------------------------------------------
typedef unsigned char byte_t;


//-----------------------------------------------------------------------------
template < typename TypeAllocated >
class ObjectPool
{
public:
	void Init( const size_t numObjectsPerBlock ); //Note that init is called at start to kick off the ObjectPool, not what we alloc from per object--that's alloc().
	TypeAllocated* Allocate();
	void Delete( TypeAllocated* ptr );


private:
	byte_t* m_memoryBuffer; //A little more work if you want multiple Buffer* m_buffers (i.e. growing ObjectPool).
		//In this case, define struct Buffer { Buffer* next; } and keep a linked list with the buffer* pointing to block start.
	PageNode* m_freePagesStack; //m_freeList.
	CriticalSection m_criticalSection;
};


//--------------------------------------------------------------------------------------------------------------
template < typename TypeAllocated > void ObjectPool<TypeAllocated>::Init( const size_t numObjectsPerBlock )
{
	size_t bufferSize = numObjectsPerBlock * sizeof( TypeAllocated );

	ASSERT_OR_DIE( sizeof( TypeAllocated ) >= sizeof( PageNode ), "TypeAllocated too small, invalidating incrementing!" );
		//Can't do ++ with a PageNode since it's usually smaller than sizeof(T) since PageNode's just a single pointer in size, but can with a T++.
	TypeAllocated* buffer = (TypeAllocated*)malloc( bufferSize );
	m_memoryBuffer = (byte_t*)buffer;

	//Moving backwards through buffer so it's arranged how we want initially (will fragment with alloc/free's over time).
	for ( size_t pageIndex = numObjectsPerBlock - 1; ; pageIndex-- )
	{
		TypeAllocated* ptr = &buffer[ pageIndex ];
		PageNode* node = (PageNode*)ptr;

		node->next = m_freePagesStack; //Whatever was in front of it.
		m_freePagesStack = node; //Keeps setting it until m_freePagesStack points to the "end" of allocations, which is the front since we're looping backward.

		if ( pageIndex == 0 )
			break;
	}
}


//--------------------------------------------------------------------------------------------------------------
template < typename TypeAllocated > TypeAllocated* ObjectPool<TypeAllocated>::Allocate()
{
	TypeAllocated* newObj = (TypeAllocated*)m_freePagesStack;
	m_freePagesStack = m_freePagesStack->next;
	new ( newObj ) TypeAllocated();

	return newObj;
}


//--------------------------------------------------------------------------------------------------------------
template < typename TypeAllocated > void ObjectPool<TypeAllocated>::Delete( TypeAllocated* ptr )
{
	if ( ptr == nullptr )
		return;

	ptr->~TypeAllocated();
	PageNode* n = (PageNode*)ptr;
	n->next = m_freePagesStack;
	m_freePagesStack = n; //This is why even if we alloc 3 things, and free the middle, things stay sane.
}