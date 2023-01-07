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
}



// Must be at the end of the file
#include "enum_helper.h"
