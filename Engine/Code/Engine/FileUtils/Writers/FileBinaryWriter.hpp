#pragma once


#include "Engine/FileUtils/Writers/BinaryWriter.hpp"
#include <stdio.h>


class FileBinaryWriter : public BinaryWriter
{
public:
	FileBinaryWriter( EndianMode endianMode = LITTLE_ENDIAN ) : BinaryWriter( endianMode ) {}
	bool open( const char* fileName, bool append = false )
	{
		const char* accessMode = append ? "ab" : "wb";
			//"ab": Append + Binary modes (adds to end of file).
			//"wb": Write + Binary mode (new file) (overwrites file).

		errno_t error = fopen_s( &m_fileHandle, fileName, accessMode );
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
	virtual size_t WriteBytes( const void* sourceData, const size_t numBytes ) override
	{
		return fwrite( sourceData, sizeof( byte_t ), numBytes, m_fileHandle );
	}


private:
	FILE* m_fileHandle;
};