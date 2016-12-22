#pragma once
#include <string.h>


#include "Engine/Memory/ByteUtils.hpp"


class BinaryWriter abstract
{
public:
	BinaryWriter( EndianMode endianMode = LITTLE_ENDIAN ) : m_endianMode( endianMode ) {}
		//Intel CPUs are Little-endian, hence the default.
		//Wii, PS3, 360 were Big-endian; PS4, XB1 seem to be Little-endian.
	void SetEndianMode( EndianMode newMode ) { m_endianMode = newMode; }
	
	virtual size_t WriteBytes( const void* sourceData, const size_t numBytes ) = 0;
	bool WriteString( const char* string ) //Handles nullptr, empty strings "", and normal strings.
	{
		if ( nullptr == string ) //Length-zero case.
		{
			Write<uint32_t>( 0U );
			return true;
		}

		size_t contentLength = strlen( string ); //Empty string "" has 0 chars, but buffer length 1.
		size_t bufferLength = contentLength + 1; //So this here handles both normal and empty strings!

		//No need to worry about endian mode of a character array, because sizeof(char) == 1 byte!
			//Unless we're doing UTF-16, that is, but we're doing UTF-8 here.
			//In Professor Forseth's words, "yet another reason UTF-8 is the superior encoding :)".

		//Write the size and then the content.
		return Write<uint32_t>( bufferLength ) && ( WriteBytes( string, bufferLength ) == bufferLength );
	}
	template <typename ArrayDataType> bool Write( const ArrayDataType& value )
	{
		ArrayDataType dataCopy = value;
		size_t dataSize = sizeof( ArrayDataType );
		
		if ( GetLocalMachineEndianness() != m_endianMode )
			ByteSwap( &dataCopy, dataSize );

		unsigned int numBytesWritten = WriteBytes( &dataCopy, dataSize );
		return numBytesWritten == dataSize;
	}


private:
	EndianMode m_endianMode;
};
