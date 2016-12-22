#include "Engine/Renderer/VertexBuffer.hpp"


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#include "Engine/Renderer/OpenGLExtensions.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"


//--------------------------------------------------------------------------------------------------------------
unsigned int GetOpenGLBufferUsage( BufferUsage engineBufferUsage )
{
	switch ( engineBufferUsage )
	{
	case BufferUsage::STATIC_DRAW: return GL_STATIC_DRAW;
	case BufferUsage::DYNAMIC_DRAW: return GL_DYNAMIC_DRAW;
	case BufferUsage::STREAM_DRAW: return GL_STREAM_DRAW;
	default: ERROR_AND_DIE( "Unsupported BufferUsage in GetOpenGLBufferUsage!" );
	}
}


//--------------------------------------------------------------------------------------------------------------
VertexBuffer::VertexBuffer( unsigned int numElements, unsigned int sizeOfElementInBytes, BufferUsage usage, const void* data )
	: m_numElements( numElements )
	, m_elementSize( sizeOfElementInBytes )
	, m_bufferUsage( usage )
{
	glGenBuffers( 1, &m_bufferID );

	UpdateBuffer( data, numElements, sizeOfElementInBytes, usage );
}


//--------------------------------------------------------------------------------------------------------------
VertexBuffer::~VertexBuffer()
{
	glDeleteBuffers( 1, &m_bufferID );
}


//--------------------------------------------------------------------------------------------------------------
void VertexBuffer::UpdateBuffer( const void* data, unsigned int numElements, unsigned int sizeOfElementInBytes, BufferUsage bufferUsage /*=USE_LAST_USAGE=*/ )
{
	m_elementSize = sizeOfElementInBytes;
	m_numElements = numElements;
	if ( bufferUsage != USE_LAST_USAGE )
		m_bufferUsage = bufferUsage;

	glBindBuffer( GL_ARRAY_BUFFER, m_bufferID );

	glBufferData( GL_ARRAY_BUFFER, GetBufferSize(), data, GetOpenGLBufferUsage( m_bufferUsage ) );

	glBindBuffer( GL_ARRAY_BUFFER, NULL );
}
