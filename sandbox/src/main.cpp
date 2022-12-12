#include "azhalCommon.h"
#include "azhalRenderer.h"

Int32 main( int argc, char** argv )
{
	using namespace azhal;

	AzhalLogger::Init( "Azhal" );
	AZHAL_LOG_INFO( "Initialized logger.." );

	cxxopts::Options cmd_line_options( "Azhal", "A vulkan renderer" );
	cmd_line_options.add_options()
		( "vkValidation", "enable vulkan api validation" )
		( "gpuValidation", "enable gpu-assisted validation" );

	const cxxopts::ParseResult& cmd_line_result = cmd_line_options.parse( argc, argv );

	WindowPtr pSandboxWindow = std::make_unique<Window>( "Azhal Sandbox", 1280, 720 );

	const RendererCreateInfo renderer_create_info
	{
		.pWindow = pSandboxWindow,
		.IsValidationLayersEnabled = cmd_line_result.count( "vkValidation" ) > 0,
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