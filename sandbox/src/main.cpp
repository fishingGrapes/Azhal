#include "azhalCommon.h"
#include "azhalRenderer.h"

Int32 main()
{
	AzhalLogger::Init( "Azhal" );
	AZHAL_LOG_INFO( "Initialized logger" );

	using namespace azhal;

	WindowPtr pSandboxWindow = std::make_unique<Window>( "Azhal Sandbox", 1280, 720 );

	RendererCreateInfo renderer_create_info
	{
		.pWindow = pSandboxWindow,
		.IsValidationLayersEnabled = true,
		.IsGpuAssistedValidationEnabled = false,
		.DebugMessageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
	};
	RendererPtr pRenderer = std::make_unique<Renderer>( renderer_create_info );

	do
	{
		// update loop
	} while( pSandboxWindow->Poll() );

	return 0;
}