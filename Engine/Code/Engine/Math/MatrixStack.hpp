#pragma once


#include "Engine/Math/Matrix4x4.hpp"
#include <stack>


template <typename T> class MatrixStack
{
public:
	MatrixStack( Ordering ordering ) {	m_transforms.push( T( ordering ) ); }
	void Push( const T& newTransform );
	void Pop() { if ( !IsEmpty() ) m_transforms.pop(); }
	T Peek() const { return m_transforms.top(); }
	bool IsEmpty() const { return m_transforms.size() <= 1; }
	unsigned int GetCount() const {	return m_transforms.size(); }

private:
	std::stack<T> m_transforms;
};

typedef MatrixStack<Matrix4x4f> Matrix4x4Stack;


//-----------------------------------------------------------------------------
template <typename T> void MatrixStack<T>::Push( const T& newTransform )
{
	T top = Peek();
	T newTransformCopy = newTransform;

	//T newTop = newTransform * top; //If engine multiplies vectors on the left (v*T).
	//T newTop = top * newTransform; //If engine multiplies vectors on the right (T*v).

	m_transforms.push( newTransformCopy * top );
}
