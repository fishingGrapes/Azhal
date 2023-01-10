#pragma once

#define TO_STRING(x) #x

#define AZHAL_INLINE __inline
#define AZHAL_FORCE_INLINE __forceinline
#define AZHAL_NO_INLINE  __declspec(noinline)
#define AZHAL_DEBUG_BREAK __debugbreak

#define AZHAL_FILE __FILE__
#define AZHAL_FUNCTION __FUNCTION__
#define AZHAL_LINE __LINE__	

#define VK_SIZE_CAST(x) static_cast<Uint32>(x)

// this is required to take into account the project being placed under temp/build/
#define AZHAL_FILE_PATH(path) "../../../" path