#pragma once


#if defined(AZHAL_DEBUG) || defined(AZHAL_RELEASE)

#define AZHAL_STATIC_ASSERT(eval, msg) static_assert(eval, msg)
#define AZHAL_ASSERT(eval, msg) assert(((void)msg, eval))

#else

#define AZHAL_STATIC_ASSERT(eval, msg)
#define AZHAL_ASSERT(eval, msg)

#endif