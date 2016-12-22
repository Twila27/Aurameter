#include "Engine/FileUtils/FileUtils.hpp"
#include "Engine/String/StringUtils.hpp"
#include <cstdio>
#include <io.h>


//--------------------------------------------------------------------------------------------------------------
bool TryCreateFile( FILE** file, const char* filename, const char* ioMode )
{
	errno_t err = fopen_s( file, filename, ioMode );
	return ( err == 0 );
}


//--------------------------------------------------------------------------------------------------------------
bool LoadBinaryFileIntoBuffer( const std::string& filePath, std::vector< unsigned char >& out_buffer )
{
	FILE* file;
	errno_t err = fopen_s(&file, filePath.c_str(), "rb");
	if ( err != 0 ) return false;
	else
	{
		//Resize vector buffer size to actual file size.
		fseek( file, 0, SEEK_END ); //Move pointer to 0 from end of file.
		size_t fileSize = ftell( file );
		out_buffer.resize( fileSize );

		rewind( file ); //Back to start for fread.
		fread( &out_buffer[ 0 ], sizeof(unsigned char), fileSize, file );

		fclose( file );
	}
	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool LoadFloatsFromTextFileIntoBuffer( const std::string& filePath, std::vector< float >& out_buffer )
{
	FILE* file;
	errno_t err = fopen_s( &file, filePath.c_str(), "rb" );
	if ( err != 0 ) return false;
	else
	{
		float tmpFloat;
		while ( !feof( file ) )
		{
			fscanf_s( file, "%f\n", &tmpFloat );
			out_buffer.push_back( tmpFloat );
		}

		fclose( file );
	}
	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool SaveBufferToBinaryFile( const std::string& filePath, const std::vector< unsigned char>& buffer )
{
	FILE* file;
	errno_t err = fopen_s( &file, filePath.c_str(), "wb" );
	if ( err != 0 ) return false;
	else
	{
		fwrite( &buffer[ 0 ], 1, buffer.size(), file );

		fclose( file );
	}
	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool SaveFloatsToTextFile( const std::string& filePath, const std::vector < float > & buffer )
{
	FILE* file;
	errno_t err = fopen_s( &file, filePath.c_str(), "wb" );
	if ( err != 0 ) return false;
	else
	{
		for ( int bufferIndex = 0; bufferIndex < (int)buffer.size(); bufferIndex++ )
		{
			fprintf( file, "%f\n", buffer[ bufferIndex ] );
		}

		fclose( file );
	}
	return true;
}


//--------------------------------------------------------------------------------------------------------------
//Credit SD4 Prof. Ken Harward
std::vector< std::string > EnumerateFilesInDirectory( const std::string& relativeDirectoryPath, const std::string& filePattern )
{
	std::string					searchPathPattern = relativeDirectoryPath + "/" + filePattern;
	std::vector< std::string > foundFiles;

	int error = 0;
	struct _finddata_t fileInfo;
	intptr_t searchHandle = _findfirst( searchPathPattern.c_str(), &fileInfo );
	while ( searchHandle != -1 && !error )
	{
		std::string relativePathToFile = Stringf( "%s/%s", relativeDirectoryPath.c_str(), fileInfo.name );
		bool		isDirectory = fileInfo.attrib & _A_SUBDIR ? true : false;
		bool		isHidden = fileInfo.attrib & _A_HIDDEN ? true : false;

		if ( !isDirectory && !isHidden )
			foundFiles.push_back( relativePathToFile );

		error = _findnext( searchHandle, &fileInfo );
	}
	_findclose( searchHandle );

	return foundFiles;
}


//--------------------------------------------------------------------------------------------------------------
//Modded off EnumerateFilesInDirectory.
unsigned int CountFilesInDirectory( const std::string& relativeDirectoryPath, const std::string& filePattern )
{
	std::string					searchPathPattern = relativeDirectoryPath + "/" + filePattern;
	unsigned int foundFiles = 0;

	int error = 0;
	struct _finddata_t fileInfo;
	intptr_t searchHandle = _findfirst( searchPathPattern.c_str(), &fileInfo );
	while ( searchHandle != -1 && !error )
	{
		std::string relativePathToFile = Stringf( "%s/%s", relativeDirectoryPath.c_str(), fileInfo.name );
		bool		isDirectory = fileInfo.attrib & _A_SUBDIR ? true : false;
		bool		isHidden = fileInfo.attrib & _A_HIDDEN ? true : false;

		if ( !isDirectory && !isHidden )
			++foundFiles;

		error = _findnext( searchHandle, &fileInfo );
	}
	_findclose( searchHandle );

	return foundFiles;
}