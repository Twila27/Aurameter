#pragma once


template < typename InPlaceLinkedListNodeType >
bool IsEmpty( InPlaceLinkedListNodeType node ) { return node == nullptr; } //?


//--------------------------------------------------------------------------------------------------------------
template < typename InPlaceLinkedListNodeType >
//Add newChildNode to the children of parentNode (the pointer to the children == parentChildrenNode).
void Append( InPlaceLinkedListNodeType& parentChildrenNode, InPlaceLinkedListNodeType& newChildNode )
{
	if ( parentChildrenNode == nullptr )
	{
		parentChildrenNode = newChildNode;
	}
	else
	{
		InPlaceLinkedListNodeType currentChild = parentChildrenNode;

		//Move to the end of parentNode's children, and link in newChildNode.
		while ( currentChild->next != nullptr )
			currentChild = currentChild->next;

		currentChild->next = newChildNode;
		newChildNode->prev = currentChild;
	}
}

// e.g. To loop through a circular in-place linked list:
// Presuming you have a node* list; //nullptr when empty, so below for loop quits immediately.
// node* iter;
// for ( iter = list; iter != nullptr; iter = ( iter->next == list ) ? nullptr : iter->next() )
// [
// 		//Do whatever you want while walking list.
// ]