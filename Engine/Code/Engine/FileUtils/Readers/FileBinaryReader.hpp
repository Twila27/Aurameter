#pragma once


#include "Engine/FileUtils/Readers/BinaryReader.hpp"
#include <stdio.h>


class FileBinaryReader : public BinaryReader
{
public:
	FileBinaryReader( EndianMode endianMode = LITTLE_ENDIAN ) : BinaryReader( endianMode ) {}
	bool open( const char* fileName )
	{
		errno_t error = fopen_s( &m_fileHandle, fileName, "rb" );
		if ( error != 0 )
			return false;

		return true;
	}
	void close()
	{
		if ( m_fileHandle != nullptr )
		{
			fclose( m_fileHandle );
			m_fileHandle = nullptr;
		}
	}
	virtual size_t ReadBytes( void* sourceData, const size_t numBytes ) override;


private:
	FILE* m_fileHandle;
};