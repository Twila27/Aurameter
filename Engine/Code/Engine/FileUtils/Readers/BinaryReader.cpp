#include "Engine/FileUtils/Readers/BinaryReader.hpp"

bool BinaryReader::ReadString( std::string& out_string )
{
	//Because we wrote a length and the buffer, we need to read a length, then the buffer.
	size_t bufferLength;
	if ( false == Read<uint32_t>( &bufferLength ) )
		return false;

	out_string.resize( bufferLength );// = new char[ bufferLength ];

									  //Write the size and then the content.
	return ReadBytes( (void*)out_string.data(), bufferLength ) == bufferLength;
}