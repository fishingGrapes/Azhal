#include "common.h"
#include "azhal_renderer.h"

Int32 main( int argc, char** argv )
{
	AzhalLogger::Init( "azhal" );
	AZHAL_LOG_INFO( "initialized logger.." );

	cxxopts::Options cmd_line_options( "Azhal", "A vulkan renderer" );
	cmd_line_options.add_options()
		( "vkValidation", "enable vulkan api validation" )
		( "gpuValidation", "enable gpu-assisted validation" );

	const cxxopts::ParseResult& cmd_line_result = cmd_line_options.parse( argc, argv );

	const Bool are_validation_layers_enabled = cmd_line_result.count( "vkValidation" ) > 0;
	const Bool is_gpu_assisted_validation_enabled = cmd_line_result.count( "gpuValidation" ) > 0;

	try
	{
		std::unique_ptr<Window> p_window = std::make_unique<Window>( "Azhal Sandbox", 1280, 720 );

		const Uvec2 framebuffer_size = p_window->get_framebuffer_size();
		const vk::Extent2D swapchain_extent { framebuffer_size.x, framebuffer_size.y };

		const gdevice::GDeviceInitParams gdevice_init_params
		{
			.pWindow = p_window->get(),
			.swapchainExtent = swapchain_extent,
			.areValidationLayersEnabled = are_validation_layers_enabled,
			.debugMessageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose,
			.isGpuAssistedValidationEnabled = is_gpu_assisted_validation_enabled
		};
		gdevice::init( gdevice_init_params );

		do
		{
			gdevice::update();
		} while( p_window->poll() );

		gdevice::shutdown();
	}
	catch( GDeviceException& e )
	{
		AZHAL_LOG_ALWAYS_ENABLED( "[GDeviceException] {0}", e.what() );
	}

	return 0;
}