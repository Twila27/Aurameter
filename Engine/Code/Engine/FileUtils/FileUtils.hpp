#pragma once


#include <string>
#include <vector>


//-----------------------------------------------------------------------------
bool TryCreateFile( FILE** file, const char* filename, const char* ioMode );
bool LoadBinaryFileIntoBuffer( const std::string& filePath, std::vector< unsigned char >& out_buffer );
bool LoadFloatsFromTextFileIntoBuffer( const std::string& filePath, std::vector< float >& out_buffer );
bool SaveBufferToBinaryFile( const std::string& filePath, const std::vector< unsigned char >& buffer );
bool SaveFloatsToTextFile( const std::string& filePath, const std::vector < float > & buffer );
std::vector< std::string > EnumerateFilesInDirectory( const std::string& relativeDirectoryPath, const std::string& filePattern );
unsigned int CountFilesInDirectory( const std::string& relativeDirectoryPath, const std::string& filePattern );