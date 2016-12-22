#include "Engine/Memory/CBuffer.hpp"


//--------------------------------------------------------------------------------------------------------------
void CBuffer::Initialize( void* bufferData, size_t bufferMaxSize )
{
	maxSizeBytes = bufferMaxSize;
	buffer = (byte_t*)bufferData;
	writeHeadOffsetFromStart = 0;
	readHeadOffsetFromStart = 0;
}
