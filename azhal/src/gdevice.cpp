#include "azpch.h"
#include "gdevice.h"

#include "command_buffer.h"
#include "enums.h"
#include "pso.h"
#include "swapchain.h"
#include "vulkan_init_helper.h"
#include "vulkan_sync_utils.h"
#include "window.h"

#include "imgui_impl.h"

gdevice::PSO pso;
vk::Semaphore imageAvailableSemaphore, renderFinishedSemaphore;
vk::Fence inflightFence;

vk::Queue graphicsQueue;
vk::Queue presentQueue;

vk::CommandBuffer cmd_buffer;

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
		pso = create_pso( s_device, pso_creation_params );

		const vk::ResultValue rv_image_avail_semaphore = s_device.createSemaphore( {} );
		imageAvailableSemaphore = get_vk_result( rv_image_avail_semaphore );
		const vk::ResultValue rv_render_finish_semaphore = s_device.createSemaphore( {} );
		renderFinishedSemaphore = get_vk_result( rv_render_finish_semaphore );

		const vk::FenceCreateInfo fence_create_info
		{
			.flags = vk::FenceCreateFlagBits::eSignaled
		};
		const vk::ResultValue rv_inflight_fence = s_device.createFence( fence_create_info );
		inflightFence = get_vk_result( rv_inflight_fence );

		graphicsQueue = s_device.getQueue( graphics_queue_family_index, 0 );
		presentQueue = s_device.getQueue( present_queue_family_index, 0 );

		cmd_buffer = allocate_command_buffer( s_device, QueueType::eGraphics );

		const ImguiInitParams imgui_init_params
		{
			.pWindow = gdevice_init_params.pWindow,
			.instance = s_instance,
			.physicalDevice = s_physicalDevice,
			.device = s_device,
			.graphicsQueueFamilyIndex = graphics_queue_family_index,
			.graphicsQueue = graphicsQueue,
			.swapchainImageFormat = s_swapchain.imageFormat,
			.swapchainImageCount = VK_SIZE_CAST( s_swapchain.images.size() )
		};
		init_imgui( imgui_init_params );
	}


	void update()
	{
		begin_imgui();

		const vk::Result res_fence_wait = s_device.waitForFences( inflightFence, VK_TRUE, UINT32_MAX );
		vk::resultCheck( res_fence_wait, "" );

		const vk::Result res_rest_fences = s_device.resetFences( inflightFence );
		vk::resultCheck( res_rest_fences, "" );

		const vk::ResultValue rv_image_index = s_device.acquireNextImageKHR( s_swapchain.vkSwapchain, UINT32_MAX, imageAvailableSemaphore );
		Uint32 image_index = get_vk_result( rv_image_index );


		cmd_buffer.reset();

		vk::CommandBufferBeginInfo cmd_buffer_begin_info {};
		vk::Result res_cmd_buffer_begin = cmd_buffer.begin( cmd_buffer_begin_info );
		vk::resultCheck( res_cmd_buffer_begin, "failed to begin command buffer" );

		insert_image_pipeline_barrier( cmd_buffer, s_swapchain.images[ image_index ],
			vk::ImageLayout::eUndefined, AccessTypeBits::eAccessTypeInvalid,
			vk::ImageLayout::eColorAttachmentOptimal, AccessTypeBits::eAccessTypeWrite );

		const vk::RenderingAttachmentInfo color_attachment_info
		{
			.imageView = s_swapchain.imageViews[ image_index ],
			.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eStore,
			.clearValue = std::array<Float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f }
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

		cmd_buffer.beginRendering( rendering_info );
		{
			cmd_buffer.bindPipeline( vk::PipelineBindPoint::eGraphics, pso.vkPipelineObject );

			const vk::Viewport viewport { 0.0f, 0.0f, static_cast< Float >( s_swapchain.imageExtent.width ), static_cast< Float >( s_swapchain.imageExtent.height ), 0.0f, 1.0f };
			const vk::Rect2D scissor { {0, 0}, s_swapchain.imageExtent };

			cmd_buffer.setViewport( 0, viewport );
			cmd_buffer.setScissor( 0, scissor );

			cmd_buffer.draw( 3, 1, 0, 0 );

			end_imgui( cmd_buffer );
		}
		cmd_buffer.endRendering();

		insert_image_pipeline_barrier( cmd_buffer, s_swapchain.images[ image_index ],
			vk::ImageLayout::eColorAttachmentOptimal, AccessTypeBits::eAccessTypeWrite,
			vk::ImageLayout::ePresentSrcKHR, AccessTypeBits::eAccessTypeInvalid );

		vk::resultCheck( cmd_buffer.end(), "" );

		const vk::PipelineStageFlags waitStageMask[ 1 ] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

		const vk::SubmitInfo submit_info
		{
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &imageAvailableSemaphore,
			.pWaitDstStageMask = waitStageMask,
			.commandBufferCount = 1,
			.pCommandBuffers = &cmd_buffer,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &renderFinishedSemaphore
		};

		vk::Result res_submit_gfx_cmd_buffer = graphicsQueue.submit( submit_info, inflightFence );
		vk::resultCheck( res_submit_gfx_cmd_buffer, "failed to submit gfx command buffer" );

		const vk::PresentInfoKHR present_info
		{
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &renderFinishedSemaphore,
			.swapchainCount = 1,
			.pSwapchains = &s_swapchain.vkSwapchain,
			.pImageIndices = &image_index
		};

		vk::Result res_submit_present = presentQueue.presentKHR( present_info );
		vk::resultCheck( res_submit_present, "failed to submit present command buffer" );

	}


	void shutdown()
	{
		vk::Result res_device_wait_idle = s_device.waitIdle();
		vk::resultCheck( res_device_wait_idle, "" );

		shutdown_imgui( s_device );
		
		free_command_buffer( s_device, QueueType::eGraphics, cmd_buffer );

		s_device.destroy( imageAvailableSemaphore );
		s_device.destroy( renderFinishedSemaphore );
		s_device.destroy( inflightFence );
		destroy_pso( s_device, pso );

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