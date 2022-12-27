#pragma once
#include "typedefs.h"

[[nodiscard( "common::fileIO::LoadBinaryBlob" )]]
ByteBufferDynamic LoadBinaryBlob( const AnsiChar* file_path );