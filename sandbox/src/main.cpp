#include "common.h"
#include "azhalRenderer.h"

Int32 main( int argc, char** argv )
{
	using namespace azhal;

	AzhalLogger::Init( "azhal" );
	AZHAL_LOG_INFO( "initialized logger.." );

	cxxopts::Options cmd_line_options( "Azhal", "A vulkan renderer" );
	cmd_line_options.add_options()
		( "vkValidation", "enable vulkan api validation" )
		( "gpuValidation", "enable gpu-assisted validation" );

	const cxxopts::ParseResult& cmd_line_result = cmd_line_options.parse( argc, argv );

	try
	{
		WindowPtr pSandboxWindow = std::make_unique<Window>( "Azhal Sandbox", 1280, 720 );

		const RendererCreateInfo renderer_create_info
		{
			.pWindow = pSandboxWindow,
			.IsValidationLayersEnabled = cmd_line_result.count( "vkValidation" ) > 0,
			.IsGpuAssistedValidationEnabled = cmd_line_result.count( "gpuValidation" ) > 0,
			.DebugMessageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
		};
		RendererPtr pRenderer = std::make_unique<Renderer>( renderer_create_info );

		do
		{
			// update loop
		} while( pSandboxWindow->Poll() );
	}
	catch( AzhalException& e )
	{
		AZHAL_LOG_ALWAYS_ENABLED( "[AzhalException] {0}", e.what() );
	}

	return 0;
}