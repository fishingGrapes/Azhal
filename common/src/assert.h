#pragma once
#include <assert.h>

#if defined(AZHAL_DEBUG) || defined(AZHAL_RELEASE)

#define AZHAL_STATIC_ASSERT(eval, msg) static_assert(eval, msg)
#define AZHAL_ASSERT(eval, msg) assert(((void)msg, eval))

#define AZHAL_NULL_CHECK(x)                                                         \
do                                                                                  \
{                                                                                   \
	if( !x ) { AZHAL_LOG_CRITICAL( "failed null pointer assert" ); __debugbreak(); }\
} while ( false );                                                                  \

#else

#define AZHAL_STATIC_ASSERT(eval, msg)
#define AZHAL_ASSERT(eval, msg)
#define AZHAL_NULL_CHECK(x)

#endif

#define AZHAL_FATAL_ASSERT(eval, msg) assert(((void)msg, eval))