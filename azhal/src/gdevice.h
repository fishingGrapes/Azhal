#pragma once
#include "swapchain.h"

#include "command_buffer.h"
#include "enums.h"
#include "pso.h"
#include "swapchain.h"
#include "window.h"

namespace gdevice
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct GDeviceInitParams
	{
		void* pWindow = nullptr;
		vk::Extent2D swapchainExtent;

		Bool areValidationLayersEnabled = false;
		vk::DebugUtilsMessageSeverityFlagBitsEXT debugMessageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;

		Bool isGpuAssistedValidationEnabled = false;
	};

	struct Context : NonCopyable
	{
	public:
		Context( Context&& other )
			: instance( other.instance )
			, instanceDynamicDispatchLoader( other.instanceDynamicDispatchLoader )
			, debugMessenger( other.debugMessenger )
			, surface( other.surface )
			, physicalDevice( other.physicalDevice )
			, device( other.device )
			, swapchain( std::move( other.swapchain ) )
		{

			other.instance = VK_NULL_HANDLE;
			other.instanceDynamicDispatchLoader = VK_NULL_HANDLE;
			other.debugMessenger = VK_NULL_HANDLE;
			other.surface = VK_NULL_HANDLE;
			other.physicalDevice = VK_NULL_HANDLE;
			other.device = VK_NULL_HANDLE;
		}

		Context() = delete;

		//-------------------------------------------------------------------//

	private:

		explicit Context( vk::Instance instance_, vk::DispatchLoaderDynamic instance_dispatch_dynamic, vk::DebugUtilsMessengerEXT debug_messenger,
			vk::SurfaceKHR surface_, vk::PhysicalDevice physical_device, vk::Device logical_device, Swapchain& swapchain_ )
			: instance( instance_ )
			, instanceDynamicDispatchLoader( instance_dispatch_dynamic )
			, debugMessenger( debug_messenger )
			, surface( surface_ )
			, physicalDevice( physical_device )
			, device( logical_device )
			, swapchain( std::move( swapchain_ ) )
		{
		}

		//-------------------------------------------------------------------//

		vk::Instance instance;
		vk::DispatchLoaderDynamic instanceDynamicDispatchLoader;
		vk::DebugUtilsMessengerEXT debugMessenger;

		vk::SurfaceKHR surface;

		vk::PhysicalDevice physicalDevice;
		vk::Device device;

		Swapchain swapchain;

		//-------------------------------------------------------------------//

		friend Context init( const GDeviceInitParams& gdevice_init_params );
		friend void shutdown( Context& gctx );

		friend void recreate_swapchain( Context& gctx, const vk::Extent2D& desired_extent );
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Context init( const GDeviceInitParams& gdevice_init_params );
	void shutdown( Context& gctx );


	AZHAL_INLINE void recreate_swapchain( Context& gctx, const vk::Extent2D& new_extent )
	{
		gctx.swapchain = recreate_swapchain( gctx.physicalDevice, gctx.device, gctx.surface, new_extent, gctx.swapchain );
	}

}