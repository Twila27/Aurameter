#pragma once

//-----------------------------------------------------------------------------
typedef unsigned char byte_t;
typedef unsigned __int32 uint32_t;


//-----------------------------------------------------------------------------
enum EndianMode
{
	LITTLE_ENDIAN = 0, //Good for bit-packing, you want to write LSByte or "little end" first.
	BIG_ENDIAN //Write the LSByte last, the "big end" first.
};

/* ENDIANESS == BYTE ORDER, NOT BIT ORDER.
	* int32_t value = 0x0102'0304; //Remember this means 0x04 is the least significant bit. Think where 0 is in binary-decimal conversion.
	* To see the layout in memory, convert value data to an array: byte_t* array = (byte_t*)&value;
	* So if array[ 0 ] == 0x04, Little - Endian -- the "little end (least significant bit) first".
	* i.e. [0x04][0x03][0x02][0x01] in memory for LITTLE endian.
*/


//--------------------------------------------------------------------------------------------------------------
extern EndianMode GetLocalMachineEndianness();



//--------------------------------------------------------------------------------------------------------------
// static inline void GetLocalMachineEndiannessWithHTONL() //Commented out to eliminate socket library dependency.
// {
// 	if ( htonl( 1 ) == 1 ) //Exists in socket library--"host network long" converts from host to network (always big) endianness.
// 		return BIG_ENDIAN;
// 	else
// 		return LITTLE_ENDIAN;
// }

//-----------------------------------------------------------------------------

//!\ For a struct { int a, b, c; } you would have to do ByteSwap(a); ByteSwap(b); ByteSwap(c); NOT ByteSwap(theStruct).
static inline void ByteSwap( void* data, const size_t dataSize )
{
	byte_t* dataArray = (byte_t*)data;
	unsigned int lastIndex = dataSize - 1;
	unsigned int midpointIndex = dataSize / 2;

	//Only iterate halfway through, else we'd swap and swap back!
	for ( unsigned int byteIndex = 0; byteIndex < midpointIndex; byteIndex++ )
	{
		byte_t temp = dataArray[ byteIndex ];
		dataArray[ byteIndex ] = dataArray[ lastIndex - byteIndex ];
		dataArray[ lastIndex - byteIndex ] = temp;
	}
}
