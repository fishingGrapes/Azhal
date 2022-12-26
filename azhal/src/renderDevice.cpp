#include "azpch.h"
#include "renderDevice.h"

#include <GLFW/glfw3.h>
#include "vulkanHelper.h"
#include "enums.h"

namespace azhal
{
	RenderDevice::RenderDevice( const vk::Instance& instance, const vk::SurfaceKHR& surface, const vk::DispatchLoaderDynamic& instance_ddl )
	{
		const std::vector<const AnsiChar*> required_extensions = GetRequiredDeviceExtensions();
		m_physicalDevice = GetSuitablePhysicalDevice( instance );

		const auto find_queue_family_fn = [this]( vk::QueueFlagBits queue_flag, Bool is_present_queue = false )  -> QueueFamily
		{
			const std::vector<vk::QueueFamilyProperties>& queue_family_props = m_physicalDevice.getQueueFamilyProperties();
			for( Uint32 i = 0; i < queue_family_props.size(); ++i )
			{
				if( queue_family_props[ i ].queueFlags & queue_flag )
				{
					return i;
				}
			}

			AZHAL_LOG_ALWAYS_ENABLED( "Failed to find an appropriate queue family for the given queue flag, {0}", queue_flag );
			throw AzhalException( "Failed to find queue family" );
		};

		const auto find_present_queue_family_fn = [&surface, &instance_ddl, this]()  -> QueueFamily
		{
			const std::vector<vk::QueueFamilyProperties>& queue_family_props = m_physicalDevice.getQueueFamilyProperties();
			for( Uint32 i = 0; i < queue_family_props.size(); ++i )
			{
				const vk::ResultValue rv_surface_support = m_physicalDevice.getSurfaceSupportKHR( i, surface, instance_ddl );
				if( rv_surface_support.result == vk::Result::eSuccess )
				{
					return rv_surface_support.value;
				}
			}

			AZHAL_LOG_ALWAYS_ENABLED( "Failed to find  present queue family for the given queue flag" );
			throw AzhalException( "Failed to find present queue family" );
		};

		const QueueFamily graphics_queue_family = find_queue_family_fn( vk::QueueFlagBits::eGraphics );
		const QueueFamily compute_queue_family = find_queue_family_fn( vk::QueueFlagBits::eCompute );
		const QueueFamily transfer_queue_family = find_queue_family_fn( vk::QueueFlagBits::eTransfer );
		const QueueFamily present_queue_family = find_present_queue_family_fn();

		const std::set<QueueFamily> unique_queue_families
		{
			graphics_queue_family,
			compute_queue_family,
			transfer_queue_family,
			present_queue_family
		};

		std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
		for( QueueFamily queue_family : unique_queue_families )
		{
			// TODO: queue_priority
			static const Float QUEUE_PRIORITY = 1.0f;
			const vk::DeviceQueueCreateInfo queue_create_info
			{
				.queueFamilyIndex = queue_family,
				.queueCount = 1,
				.pQueuePriorities = &QUEUE_PRIORITY
			};
			queue_create_infos.emplace_back( queue_create_info );
		}

		vk::DeviceCreateInfo device_create_info
		{
			.queueCreateInfoCount = VK_SIZE_CAST( queue_create_infos.size() ),
			.pQueueCreateInfos = queue_create_infos.data(),
			.enabledExtensionCount = VK_SIZE_CAST( required_extensions.size() ),
			.ppEnabledExtensionNames = required_extensions.data(),
			// TODO: add enabled features
			.pEnabledFeatures = VK_NULL_HANDLE
		};

		const vk::ResultValue rv_device = m_physicalDevice.createDevice( device_create_info );
		m_device = CheckVkResultValue( rv_device, "Failed to create vulkan device" );
		AZHAL_LOG_WARN( "vulkan device created" );

		m_graphicsQueue = GpuQueue( m_device.getQueue( graphics_queue_family, 0 ), graphics_queue_family );
		m_computeQueue = GpuQueue( m_device.getQueue( compute_queue_family, 0 ), compute_queue_family );
		m_transferQueue = GpuQueue( m_device.getQueue( transfer_queue_family, 0 ), transfer_queue_family );
		m_presentQueue = GpuQueue( m_device.getQueue( present_queue_family, 0 ), present_queue_family );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Swapchain RenderDevice::CreateSwapchain( const WindowPtr& pWindow, const vk::SurfaceKHR& surface )
	{
		return Swapchain( pWindow, m_physicalDevice, m_device, surface );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	vk::PhysicalDevice RenderDevice::GetSuitablePhysicalDevice( const vk::Instance& instance ) const
	{
		const vk::ResultValue rv_physical_devices = instance.enumeratePhysicalDevices();
		const std::vector<vk::PhysicalDevice>& physical_devices = CheckVkResultValue( rv_physical_devices, "failed to enumerate physical devices" );

		// TODO: check for appropriate physical device props
		const vk::PhysicalDevice selected_physical_device = physical_devices[ 0 ];
		const vk::PhysicalDeviceProperties& physical_device_props = selected_physical_device.getProperties();

		return selected_physical_device;
	}

	std::vector<const AnsiChar*> RenderDevice::GetRequiredDeviceExtensions() const
	{
		const std::vector<const AnsiChar*> required_device_extensions
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		return required_device_extensions;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
}