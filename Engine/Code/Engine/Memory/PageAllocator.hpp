#pragma once


//--------------------------------------------------------------------------------------------------------------
struct PageNode
{
	PageNode* next;
};



//--------------------------------------------------------------------------------------------------------------
class PageAllocator
{
public:
	PageAllocator( size_t pageSize, size_t initialNumPages );
	void* Allocate();
	void Free( void* ptr );

private:
	PageNode* m_freePagesStack; //Since we "pop" them off on free(), and "push" them on in alloc().
};
