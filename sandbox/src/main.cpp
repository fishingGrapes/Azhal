#include "azhalCommon.h"

Int32 main()
{
	CAzhalLog::Init( "Azhal" );
	AZHAL_LOG_INFO( "Initialized logger" );

	AZHAL_ASSERT( 2 == 3, "Must fail this" );
	return 0;
}