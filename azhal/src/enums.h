#pragma once

namespace gdevice
{
	enum class QueueType : Uint32
	{
		eInvalid = 0,
		eGraphics = 1,
		eCompute = 2,
		ePresent = 3,
		eTransfer = 4
	};

	enum AccessTypeBits : Uint32
	{
		eAccessTypeRead = 0x00000001,
		eAccessTypeWrite = 0x00000002,
		eAccessTypeReadWrite = ( eAccessTypeRead | eAccessTypeWrite )
	};
}



// Must be at the end of the file
#include "enum_helper.h"
