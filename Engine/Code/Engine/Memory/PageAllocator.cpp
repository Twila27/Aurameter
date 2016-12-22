#include "Engine/Memory/PageAllocator.hpp"
#include "Engine/EngineCommon.hpp"


//--------------------------------------------------------------------------------------------------------------
PageAllocator::PageAllocator( size_t pageSize, size_t initialNumPages )
{
	//WARNING: pageSize MUST be bigger than the size of a Page, which is sizeof(Page), which is sizeof(Page* next)--a pointer (size_t).
	size_t totalSize = pageSize * initialNumPages;
	byte_t* memoryBlock = (byte_t*)malloc( totalSize );
	m_freePagesStack = nullptr;
	
	//Moving backwards through buffer so it's arranged how we want initially (will fragment with alloc/free's over time).
	for ( size_t pageIndex = initialNumPages - 1; ; pageIndex-- )
	{
		byte_t* pageAsBytePtr = memoryBlock + ( pageSize * pageIndex );
		PageNode* node = (PageNode*)pageAsBytePtr;

		//TRICK:
		node->next = m_freePagesStack; //Whatever was in front of it.
		m_freePagesStack = node; //Keeps setting it until m_freePagesStack points to the "end" of allocations, which is the front since we're looping backward.

		if ( pageIndex == 0 )
			break;
	}

}


//--------------------------------------------------------------------------------------------------------------
void* PageAllocator::Allocate()
{
	PageNode* page = m_freePagesStack;
	m_freePagesStack = m_freePagesStack->next;
	return page; //When it's used, this is no longer a "page", it's an allocation we're casting.
}


//--------------------------------------------------------------------------------------------------------------
void PageAllocator::Free( void* ptr )
{
	PageNode* temp = m_freePagesStack;
	m_freePagesStack = (PageNode*)ptr;
	m_freePagesStack->next = temp; //This is why even if we alloc 3 things, and free the middle, things stay sane.
}
