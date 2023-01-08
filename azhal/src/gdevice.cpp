#include "azpch.h"
#include "gdevice.h"

#include "command_buffer.h"
#include "enums.h"
#include "pso.h"
#include "swapchain.h"
#include "vulkan_init_helper.h"
#include "window.h"


namespace
{
	vk::Instance s_instance;
	vk::DispatchLoaderDynamic s_instanceDynamicDispatchLoader;
	vk::DebugUtilsMessengerEXT s_debugMessenger;

	vk::SurfaceKHR s_surface;

	vk::PhysicalDevice s_physicalDevice;
	vk::Device s_device;

	gdevice::Swapchain s_swapchain;
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

namespace gdevice
{
	void init( const GDeviceInitParams& gdevice_init_params )
	{
		const PFN_vkDebugUtilsMessengerCallbackEXT debug_callback_fn = reinterpret_cast< PFN_vkDebugUtilsMessengerCallbackEXT >( vk_debug_callback );

		const VulkanInstanceCreationParams instance_creation_params
		{
			.enableValidationLayers = gdevice_init_params.areValidationLayersEnabled,
			.debugMessageSeverity = gdevice_init_params.debugMessageSeverity,
			.debugCallbackFn = debug_callback_fn,

			.enableGpuAssistedValidation = gdevice_init_params.isGpuAssistedValidationEnabled
		};
		s_instance = create_instance( instance_creation_params );

		s_instanceDynamicDispatchLoader = vk::DispatchLoaderDynamic( s_instance, vkGetInstanceProcAddr );

#ifdef AZHAL_ENABLE_LOGGING
		s_debugMessenger = create_debug_messenger( s_instance, gdevice_init_params.debugMessageSeverity, debug_callback_fn, s_instanceDynamicDispatchLoader );
#endif

		s_surface = create_vulkan_surface( s_instance, gdevice_init_params.pWindow );

		s_physicalDevice = get_suitable_physical_device( s_instance );

		const Uint32 graphics_queue_family_index = find_queue_family_index( s_physicalDevice, vk::QueueFlagBits::eGraphics );
		const Uint32 compute_queue_family_index = find_queue_family_index( s_physicalDevice, vk::QueueFlagBits::eCompute );
		const Uint32 transfer_queue_family_index = find_queue_family_index( s_physicalDevice, vk::QueueFlagBits::eTransfer );
		const Uint32 present_queue_family_index = find_present_queue_family_index( s_physicalDevice, s_surface, s_instanceDynamicDispatchLoader );

		{
			const std::set<Uint32> unique_queue_families
			{
				graphics_queue_family_index,
				compute_queue_family_index,
				transfer_queue_family_index,
				present_queue_family_index
			};

			s_device = create_device( s_instance, s_physicalDevice, unique_queue_families );
		}

		s_swapchain = create_swapchain( s_physicalDevice, s_device, s_surface, gdevice_init_params.swapchainExtent );

		const CommandPoolsInitParams cmd_pools_init_params
		{
			.computeQueueFamilyIndex = compute_queue_family_index,
			.graphicsQueueFamilyIndex = graphics_queue_family_index,
			.presentQueueFamilyIndex = present_queue_family_index,
			.transferQueueFamilyIndex = transfer_queue_family_index
		};
		init_command_pools( s_device, cmd_pools_init_params );

		//testing
		const PSOCreationParams pso_creation_params
		{
			.pVertexShader = AZHAL_FILE_PATH( "azhal/shaders/simple.vspv" ),
			.pFragmentShader = AZHAL_FILE_PATH( "azhal/shaders/simple.pspv" ),
			.isDynamicRendering = VK_TRUE,
			.colorAttachmentFormats = std::vector<vk::Format> { s_swapchain.imageFormat }
		};

		PSO pso = create_pso( s_device, pso_creation_params );
		vk::CommandBuffer cmd_buffer = allocate_command_buffer( s_device, QueueType::eGraphics );



		destroy_pso( s_device, pso );
	}


	void update()
	{
		vk::CommandBufferBeginInfo cmd_buffer_begin;

		const vk::RenderingAttachmentInfo color_attachment_info
		{
			//.imageView = s_swapchain.imageViews,
			.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eStore,
			.clearValue = std::array<Float, 4>{ 0.0f, 1.0f, 0.0f, 1.0f }
		};

		const vk::Rect2D render_area
		{
			.offset = {0,0},
			.extent = s_swapchain.imageExtent
		};

		const vk::RenderingInfo rendering_info
		{
			.renderArea = render_area,
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &color_attachment_info
		};

	}


	void shutdown()
	{
		destroy_command_pools( s_device );

		destroy_swapchain( s_device, s_swapchain );

		s_device.destroy();

		s_instance.destroy( s_surface );

#ifdef AZHAL_ENABLE_LOGGING
		s_instance.destroyDebugUtilsMessengerEXT( s_debugMessenger, VK_NULL_HANDLE, s_instanceDynamicDispatchLoader );
#endif
		s_instance.destroy();
	}
}