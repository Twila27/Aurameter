#include "Engine/Memory/ByteUtils.hpp"


EndianMode GetLocalMachineEndianness()
{
	union //Compiles out because it's all constant.
	{
		byte_t bdata[ 4 ];
		uint32_t uidata;
	} data;

	data.uidata = 0x04030201;

	//If first byte is the least significant, we know it's little-endian: the "little end" was first.
	return ( data.bdata[ 0 ] == 0X01 ) ? LITTLE_ENDIAN : BIG_ENDIAN;
	//End Method 2.
}