#include "Engine/FileUtils/Readers/FileBinaryReader.hpp"


size_t FileBinaryReader::ReadBytes( void* sourceData, const size_t numBytes )
{
	return fread( sourceData, sizeof( byte_t ), numBytes, m_fileHandle );
}