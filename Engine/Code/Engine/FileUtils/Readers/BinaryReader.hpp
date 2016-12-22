#pragma once
#include <string>


#include "Engine/Memory/ByteUtils.hpp"


class BinaryReader abstract
{
public:
	BinaryReader( EndianMode endianMode = LITTLE_ENDIAN ) : m_endianMode( endianMode ) {}
	   //Intel CPUs are Little-endian, hence the default.
	   //Wii, PS3, 360 were Big-endian; PS4, XB1 seem to be Little-endian.
	void SetEndianMode( EndianMode newMode ) { m_endianMode = newMode; }

	virtual size_t ReadBytes( void* out_value, const size_t numBytes ) = 0;
	bool ReadString( std::string& out_string );
	template <typename ArrayDataType> bool Read( ArrayDataType* out_value )
	{
		size_t dataSize = sizeof( ArrayDataType ); //In bytes.

		//Note will only read one of a type, not an array (for that, call Read multiple times).
		unsigned int numBytesRead = ReadBytes( out_value, dataSize );

		//Burden currently on the user to set the correct mode, not parsing the file for it.
		if ( GetLocalMachineEndianness() != m_endianMode ) 
			ByteSwap( out_value, dataSize );

		return numBytesRead == dataSize;
	}


private:
	EndianMode m_endianMode;
};
