#pragma once

#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/Memory/ByteUtils.hpp"


struct CBuffer
{
	byte_t* buffer;
	size_t writeHeadOffsetFromStart;
	size_t readHeadOffsetFromStart;
	size_t maxSizeBytes;

	void Initialize( void* bufferData, size_t bufferMaxSize );	//Just takes a preallocated buffer, does not itself allocate the memory.
	template< typename T > T* WriteToBuffer();				//Just allocates to said buffer by casting it with a template.
	template< typename T > T* ReadFromBuffer();
};


//--------------------------------------------------------------------------------------------------------------
template< typename T > T* CBuffer::WriteToBuffer()
{
	ASSERT_OR_DIE( writeHeadOffsetFromStart + sizeof( T ) <= maxSizeBytes, "Exceeded CBuffer Length!" );

	//Allocate == just reinterpretation via cast.
	T* out = (T*)( buffer + writeHeadOffsetFromStart );
	
	//Advance write head.
	writeHeadOffsetFromStart += sizeof( T );

	return out;
}


//--------------------------------------------------------------------------------------------------------------
template< typename T > T* CBuffer::ReadFromBuffer()
{
	T* out = (T*)( buffer + readHeadOffsetFromStart );

	readHeadOffsetFromStart += sizeof(T); //Note sure how else to tell sizeof to increment by!

	return out;
}
