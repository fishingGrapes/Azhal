#include "azhalCommon.h"
#include "azhalRenderer.h"

Int32 main()
{
	AzhalLogger::Init( "Azhal" );
	AZHAL_LOG_INFO( "Initialized logger" );

	using namespace azhal;

	WindowPtr pWindow = std::make_unique<Window>( "Azhal Sandbox", 1280, 720 );
	RendererPtr pRenderer = std::make_unique<Renderer>( pWindow );

	do
	{
		// update loop
	} while( pWindow->Poll() );

	return 0;
}