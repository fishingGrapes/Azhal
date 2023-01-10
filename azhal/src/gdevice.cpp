#include "azpch.h"
#include "gdevice.h"

#include "vulkan_init_helper.h"

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

namespace gdevice
{
	Context init( const GDeviceInitParams& gdevice_init_params )
	{
		const PFN_vkDebugUtilsMessengerCallbackEXT debug_callback_fn = reinterpret_cast< PFN_vkDebugUtilsMessengerCallbackEXT >( vk_debug_callback );

		const VulkanInstanceCreationParams instance_creation_params
		{
			.enableValidationLayers = gdevice_init_params.areValidationLayersEnabled,
			.debugMessageSeverity = gdevice_init_params.debugMessageSeverity,
			.debugCallbackFn = debug_callback_fn,

			.enableGpuAssistedValidation = gdevice_init_params.isGpuAssistedValidationEnabled
		};
		vk::Instance instance = create_instance( instance_creation_params );

		vk::DispatchLoaderDynamic instance_dispatch_dynamic = vk::DispatchLoaderDynamic( instance, vkGetInstanceProcAddr );

#ifdef AZHAL_ENABLE_LOGGING
		vk::DebugUtilsMessengerEXT debug_messenger = create_debug_messenger( instance, gdevice_init_params.debugMessageSeverity, debug_callback_fn, instance_dispatch_dynamic );
#endif

		vk::SurfaceKHR surface = create_vulkan_surface( instance, gdevice_init_params.pWindow );

		vk::PhysicalDevice physical_device = get_suitable_physical_device( instance );

		const Uint32 graphics_queue_family_index = find_queue_family_index( physical_device, vk::QueueFlagBits::eGraphics );
		const Uint32 compute_queue_family_index = find_queue_family_index( physical_device, vk::QueueFlagBits::eCompute );
		const Uint32 transfer_queue_family_index = find_queue_family_index( physical_device, vk::QueueFlagBits::eTransfer );
		const Uint32 present_queue_family_index = find_present_queue_family_index( physical_device, surface, instance_dispatch_dynamic );

		const std::set<Uint32> unique_queue_families
		{
			graphics_queue_family_index,
			compute_queue_family_index,
			transfer_queue_family_index,
			present_queue_family_index
		};

		vk::Device device = create_device( instance, physical_device, unique_queue_families );

		Swapchain swapchain = create_swapchain( physical_device, device, surface, gdevice_init_params.swapchainExtent );

		const CommandPoolsInitParams cmd_pools_init_params
		{
			.computeQueueFamilyIndex = compute_queue_family_index,
			.graphicsQueueFamilyIndex = graphics_queue_family_index,
			.presentQueueFamilyIndex = present_queue_family_index,
			.transferQueueFamilyIndex = transfer_queue_family_index
		};
		init_command_pools( device, cmd_pools_init_params );


		Context gctx(
			instance,
			instance_dispatch_dynamic,
			debug_messenger,
			surface,
			physical_device,
			device,
			swapchain
		);

		return gctx;
	}


	void shutdown( Context& gctx )
	{
		destroy_command_pools( gctx.device );

		destroy_swapchain( gctx.device, gctx.swapchain );

		gctx.device.destroy();

		gctx.instance.destroy( gctx.surface );

#ifdef AZHAL_ENABLE_LOGGING
		gctx.instance.destroyDebugUtilsMessengerEXT( gctx.debugMessenger, VK_NULL_HANDLE, gctx.instanceDynamicDispatchLoader );
#endif
		gctx.instance.destroy();
	}
}