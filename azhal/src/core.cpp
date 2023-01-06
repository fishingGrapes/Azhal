#include "azpch.h"
#include "core.h"

#include "enums.h"
#include "swapchain.h"
#include "vulkan_init_helper.h"
#include "window.h"


namespace
{
	vk::Instance g_instance;
	vk::DispatchLoaderDynamic g_instanceDynamicDispatchLoader;
	vk::DebugUtilsMessengerEXT g_debugMessenger;

	vk::SurfaceKHR g_surface;

	vk::PhysicalDevice g_physicalDevice;
	vk::Device g_device;

	azhal::Swapchain g_swapchain;
}

namespace
{
	VKAPI_ATTR vk::Bool32 VKAPI_CALL vk_debug_callback( vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity, vk::DebugUtilsMessageTypeFlagBitsEXT message_type,
		const vk::DebugUtilsMessengerCallbackDataEXT* p_callback_data, void* p_user_data )
	{
		String message_type_name = "";
		switch( message_type )
		{
		case vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral:
			message_type_name = "[vk_general]";
			break;
		case vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation:
			message_type_name = "[vk_validation]";
			break;
		case vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance:
			message_type_name = "[vk_performance]";
			break;
		case vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding:
			message_type_name = "[vk_deviceAddressBinding]";
			break;
		default:
			AZHAL_LOG_CRITICAL( "Invalid vk::DebugUtilsMessageTypeFlagBitsEXT flag bit: {0}", message_type );
			AZHAL_DEBUG_BREAK();
			break;
		}

		switch( message_severity )
		{
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
			AZHAL_LOG_TRACE( "{0} {1}", message_type_name, p_callback_data->pMessage );
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
			AZHAL_LOG_INFO( "{0} {1}", message_type_name, p_callback_data->pMessage );
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
			AZHAL_LOG_WARN( "{0} {1}", message_type_name, p_callback_data->pMessage );
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
			AZHAL_LOG_ERROR( "{0} {1}", message_type_name, p_callback_data->pMessage );
			break;
		default:
			AZHAL_LOG_CRITICAL( "Invalid vk::DebugUtilsMessageSeverityFlagBitsEXT flag bit: {0}", message_severity );
			AZHAL_DEBUG_BREAK();
			break;
		}

#ifdef VK_DEADLY_VALIDATION
		return VK_TRUE;
#else
		return VK_FALSE;
#endif
	}
}

namespace azhal
{
	void init( const Window& window, Bool is_validation_layers_enabled, vk::DebugUtilsMessageSeverityFlagBitsEXT debug_severity, Bool is_gpu_assisted_validation_enabled )
	{
		const PFN_vkDebugUtilsMessengerCallbackEXT& debug_callback_fn = reinterpret_cast< PFN_vkDebugUtilsMessengerCallbackEXT >( vk_debug_callback );

		const VulkanInstanceCreationParams instance_creation_params
		{
			.enableValidationLayers = is_validation_layers_enabled,
			.debugMessageSeverity = debug_severity,
			.debugCallbackFn = debug_callback_fn,

			.enableGpuAssistedValidation = is_gpu_assisted_validation_enabled
		};
		g_instance = create_instance( instance_creation_params );

		g_instanceDynamicDispatchLoader = vk::DispatchLoaderDynamic( g_instance, vkGetInstanceProcAddr );

#ifdef AZHAL_ENABLE_LOGGING
		g_debugMessenger = create_debug_messenger( g_instance, debug_severity, debug_callback_fn, g_instanceDynamicDispatchLoader );
#endif

		g_surface = create_vulkan_surface( g_instance, window.get() );

		g_physicalDevice = get_suitable_physical_device( g_instance );

		g_device = create_device( g_instance, g_physicalDevice, g_surface, g_instanceDynamicDispatchLoader );

		{
			const Uvec2 framebuffer_size = window.get_framebuffer_size();
			const vk::Extent2D desired_extent { framebuffer_size.x, framebuffer_size.y };

			g_swapchain = create_swapchain( g_physicalDevice, g_device, g_surface, desired_extent );
		}
	}


	void shutdown()
	{
		destroy_swapchain( g_device, g_swapchain );
		AZHAL_LOG_WARN( "vulkan swapchain destroyed" );

		g_device.destroy();
		AZHAL_LOG_WARN( "vulkan device destroyed" );

		g_instance.destroy( g_surface );
		AZHAL_LOG_WARN( "vulkan surface destroyed" );

#ifdef AZHAL_ENABLE_LOGGING
		g_instance.destroyDebugUtilsMessengerEXT( g_debugMessenger, VK_NULL_HANDLE, g_instanceDynamicDispatchLoader );
		AZHAL_LOG_WARN( "vulkan debug messenger destroyed" );
#endif
		g_instance.destroy();
		AZHAL_LOG_WARN( "vulkan instance destroyed" );
	}
}