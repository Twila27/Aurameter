#pragma once 

#define GET_BIT_AT_BITFIELD_INDEX(i) (1 << (i)) //2^x, or the xth bit.
#define GET_BIT_AT_BITFIELD_INDEX_MASKED( bitfield, bitIndex ) ( bitfield & GET_BIT_AT_BITFIELD_INDEX( bitIndex ) )
#define GET_BIT_WITHOUT_INDEX_MASKED( bitfield, bitValueNotAnIndex ) ( bitfield & bitValueNotAnIndex )
	//Main difference: last one already has the shifting in it.