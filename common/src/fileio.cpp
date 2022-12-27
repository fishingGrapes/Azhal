#include "fileio.h"
#include "assert.h"
#include <fstream>

ByteBufferDynamic LoadBinaryBlob( const AnsiChar* file_path )
{
	std::ifstream file_stream( file_path, std::ios::ate | std::ios::binary );
	AZHAL_ASSERT( file_stream.is_open(), "failed to open file stream for reading binary blob" );

	const Uint64 size_in_bytes = static_cast< Uint64 >( file_stream.tellg() );
	ByteBufferDynamic buffer( size_in_bytes );

	file_stream.seekg( 0 );
	file_stream.read( buffer.data(), size_in_bytes );
	file_stream.close();

	return buffer;
}
