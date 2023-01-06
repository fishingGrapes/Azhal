#include "common.h"
#include "azhal_renderer.h"

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

	const Bool is_validation_layers_enabled = cmd_line_result.count( "vkValidation" ) > 0;
	const Bool is_gpu_assisted_validation_enabled = cmd_line_result.count( "gpuValidation" ) > 0;

	try
	{
		WindowPtr p_window = std::make_unique<Window>( "Azhal Sandbox", 1280, 720 );
		azhal::init( *p_window, is_validation_layers_enabled, vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose, is_gpu_assisted_validation_enabled );

		do
		{
		} while( p_window->poll() );

		azhal::shutdown();
	}
	catch( AzhalException& e )
	{
		AZHAL_LOG_ALWAYS_ENABLED( "[AzhalException] {0}", e.what() );
	}

	return 0;
}