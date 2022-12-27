#include "azpch.h"
#include "swapchain.h"

#include <GLFW/glfw3.h>

// TODO: set appropriate values when creating swapchain
namespace azhal
{
	Swapchain::Swapchain( const WindowPtr& pWindow, const vk::PhysicalDevice& physical_device, const vk::Device& device, const vk::SurfaceKHR& surface )
		: m_format( vk::Format::eUndefined )
		, m_colorSpace( vk::ColorSpaceKHR::eSrgbNonlinear )
		, m_presentMode( vk::PresentModeKHR::eFifo )
	{
		const vk::ResultValue rv_surface_caps = physical_device.getSurfaceCapabilitiesKHR( surface );
		const vk::SurfaceCapabilitiesKHR& surface_caps = CheckVkResultValue( rv_surface_caps, "failed to get surface capabilities" );

		const vk::ResultValue rv_surface_formats = physical_device.getSurfaceFormatsKHR( surface );
		const std::vector<vk::SurfaceFormatKHR>& surface_formats = CheckVkResultValue( rv_surface_formats, "failed to get surface formats" );

		const vk::ResultValue rv_surface_present_modes = physical_device.getSurfacePresentModesKHR( surface );
		const std::vector<vk::PresentModeKHR>& surface_present_modes = CheckVkResultValue( rv_surface_present_modes, "failed to get present modes" );

		AZHAL_FATAL_ASSERT( !surface_formats.empty() && !surface_present_modes.empty(), "swapchain support is not adequate" );

		// choose format and color-space
		[&surface_formats, this]() -> void
		{
			for( const vk::SurfaceFormatKHR& format : surface_formats )
			{
				if( format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear )
				{
					m_format = vk::Format::eB8G8R8A8Srgb;
					m_colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
					return;
				}
			}

			AZHAL_LOG_ALWAYS_ENABLED( "Failed to get appropriate format and colorspace for swapchain. fallback the first one." );
			m_format = surface_formats[ 0 ].format;
			m_colorSpace = surface_formats[ 0 ].colorSpace;
		}( );

		// choose present mode
		[&surface_present_modes, this]() -> void
		{
			for( const vk::PresentModeKHR& present_mode : surface_present_modes )
			{
				if( present_mode == vk::PresentModeKHR::eMailbox )
				{
					m_presentMode = vk::PresentModeKHR::eMailbox;
					return;
				}

			}

			AZHAL_LOG_ALWAYS_ENABLED( "failed to get appropriate present mode for swapchain. fallback to FIFO mode." );
			m_presentMode = vk::PresentModeKHR::eFifo;
		}( );

		// choose swapchain extent
		[&pWindow, &surface_caps, this]() -> void
		{
			// if the current extent width is see to the special value UINT32_MAX, then the extent of the swapchain
			// can differ from the window resolution
			if( surface_caps.currentExtent.width != UINT32_MAX )
			{
				m_extent = surface_caps.currentExtent;
				return;
			}

			Int32 width = 0;
			Int32 height = 0;
			glfwGetFramebufferSize( static_cast< GLFWwindow* >( pWindow->Get() ), &width, &height );

			m_extent.width = std::clamp<Uint32>( width, surface_caps.minImageExtent.width, surface_caps.maxImageExtent.width );
			m_extent.height = std::clamp<Uint32>( height, surface_caps.minImageExtent.height, surface_caps.maxImageExtent.height );
		}( );


		Uint32 image_count = ( surface_caps.minImageCount + 1 );
		// if the maxImageCount has a special value of NULL/0, it means there is no max image count limit
		if( surface_caps.maxImageCount != NULL )
			image_count = std::min<Uint32>( image_count, surface_caps.maxImageCount );

		vk::SwapchainCreateInfoKHR vk_swapchain_create_info
		{
			.surface = surface,
			.minImageCount = image_count,
			.imageFormat = m_format,
			.imageColorSpace = m_colorSpace,
			.imageExtent = m_extent,
			.imageArrayLayers = 1,
			.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
			.imageSharingMode = vk::SharingMode::eExclusive,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = VK_NULL_HANDLE,
			.preTransform = surface_caps.currentTransform,
			.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
			.presentMode = m_presentMode,
			.clipped = VK_TRUE,
			.oldSwapchain = VK_NULL_HANDLE
		};

		const vk::ResultValue rv_swapchain = device.createSwapchainKHR( vk_swapchain_create_info );
		m_swapchain = CheckVkResultValue( rv_swapchain, "failed to create swapchain" );
		AZHAL_LOG_WARN( "vulkan swapchain created" );

		const vk::ResultValue rv_swapchain_images = device.getSwapchainImagesKHR( m_swapchain );
		m_images = CheckVkResultValue( rv_swapchain_images, "failed to retrieve swapchain images" );

		m_imageViews.reserve( image_count );
		for( const vk::Image& image : m_images )
		{
			vk::ImageViewCreateInfo image_view_create_info
			{
				.image = image,
				.viewType = vk::ImageViewType::e2D,
				.format = m_format,

				.components =
				{
					.r = vk::ComponentSwizzle::eIdentity,
					.g = vk::ComponentSwizzle::eIdentity,
					.b = vk::ComponentSwizzle::eIdentity,
					.a = vk::ComponentSwizzle::eIdentity
				},

				.subresourceRange =
				{
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				}
			};

			const vk::ResultValue rv_image_view = device.createImageView( image_view_create_info );
			const vk::ImageView image_view = CheckVkResultValue( rv_image_view, "failed to create image view from swapchain image" );

			m_imageViews.push_back( image_view );
		}
		AZHAL_LOG_WARN( "vulkan created swapchain image views" );

	}

	void Swapchain::Destroy( const vk::Device& device )
	{
		for( const vk::ImageView& image_view : m_imageViews )
		{
			device.destroy( image_view );
		}
		AZHAL_LOG_WARN( "vulkan destroyed swapchain image views" );

		device.destroy( m_swapchain );
		AZHAL_LOG_WARN( "vulkan swapchain destroyed" );
	}

}