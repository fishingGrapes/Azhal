#include "azpch.h"
#include "swapchain.h"

namespace
{
	vk::SwapchainCreateInfoKHR build_swapchain_create_info( const vk::PhysicalDevice& physical_device, const vk::SurfaceKHR& surface, vk::Extent2D desired_extent )
	{
		const vk::ResultValue rv_surface_caps = physical_device.getSurfaceCapabilitiesKHR( surface );
		const vk::SurfaceCapabilitiesKHR& surface_caps = gdevice::get_vk_result( rv_surface_caps, "failed to get surface capabilities" );

		const vk::ResultValue rv_surface_formats = physical_device.getSurfaceFormatsKHR( surface );
		const std::vector<vk::SurfaceFormatKHR>& surface_formats = gdevice::get_vk_result( rv_surface_formats, "failed to get surface formats" );

		const vk::ResultValue rv_surface_present_modes = physical_device.getSurfacePresentModesKHR( surface );
		const std::vector<vk::PresentModeKHR>& surface_present_modes = gdevice::get_vk_result( rv_surface_present_modes, "failed to get present modes" );

		AZHAL_FATAL_ASSERT( !surface_formats.empty() && !surface_present_modes.empty(), "swapchain support is not adequate" );

		// choose format and color-space
		auto [format, color_space] = [&surface_formats]() -> std::tuple<vk::Format, vk::ColorSpaceKHR>
		{
			for( const vk::SurfaceFormatKHR& format : surface_formats )
			{
				if( format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear )
				{
					return std::tuple( vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear );
				}
			}

			AZHAL_LOG_ALWAYS_ENABLED( "Failed to get appropriate format and colorspace for swapchain. fallback the first one." );
			return std::tuple( surface_formats[ 0 ].format, surface_formats[ 0 ].colorSpace );
		}( );

		// choose present mode
		vk::PresentModeKHR present_mode = [&surface_present_modes]() -> vk::PresentModeKHR
		{
			for( const vk::PresentModeKHR& present_mode : surface_present_modes )
			{
				if( present_mode == vk::PresentModeKHR::eMailbox )
				{
					return vk::PresentModeKHR::eMailbox;
				}

			}

			AZHAL_LOG_ALWAYS_ENABLED( "failed to get appropriate present mode for swapchain. fallback to FIFO mode." );
			return vk::PresentModeKHR::eFifo;
		}( );

		// choose swapchain extent
		vk::Extent2D swapchain_extent = [&desired_extent, &surface_caps]() -> vk::Extent2D
		{
			// if the current extent width is see to the special value UINT32_MAX, then the extent of the swapchain
			// can differ from the window resolution
			if( surface_caps.currentExtent.width != UINT32_MAX )
			{
				return surface_caps.currentExtent;
			}

			return {
				std::clamp<Uint32>( desired_extent.width, surface_caps.minImageExtent.width, surface_caps.maxImageExtent.width ),
				std::clamp<Uint32>( desired_extent.height, surface_caps.minImageExtent.height, surface_caps.maxImageExtent.height )
			};
		}( );


		Uint32 image_count = ( surface_caps.minImageCount + 1 );
		// if the maxImageCount has a special value of NULL/0, it means there is no max image count limit
		if( surface_caps.maxImageCount != NULL )
			image_count = std::min<Uint32>( image_count, surface_caps.maxImageCount );

		vk::SwapchainCreateInfoKHR swapchain_create_info
		{
			.surface = surface,
			.minImageCount = image_count,
			.imageFormat = format,
			.imageColorSpace = color_space,
			.imageExtent = swapchain_extent,
			.imageArrayLayers = 1,
			.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
			.imageSharingMode = vk::SharingMode::eExclusive,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = VK_NULL_HANDLE,
			.preTransform = surface_caps.currentTransform,
			.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
			.presentMode = present_mode,
			.clipped = VK_TRUE,
			.oldSwapchain = VK_NULL_HANDLE
		};

		return swapchain_create_info;
	}
}

namespace gdevice
{
	Swapchain create_swapchain( const vk::PhysicalDevice& physical_device, const vk::Device& device, const vk::SurfaceKHR& surface, vk::Extent2D desired_extent )
	{
		const vk::SwapchainCreateInfoKHR& swapchain_create_info = build_swapchain_create_info( physical_device, surface, desired_extent );

		const vk::ResultValue rv_swapchain = device.createSwapchainKHR( swapchain_create_info );
		vk::SwapchainKHR vk_swapchain = get_vk_result( rv_swapchain, "failed to create swapchain" );

		const vk::ResultValue rv_swapchain_images = device.getSwapchainImagesKHR( vk_swapchain );
		const std::vector<vk::Image>& swapchain_images = get_vk_result( rv_swapchain_images, "failed to retrieve swapchain images" );

		std::vector<vk::ImageView> swapchain_imageviews;
		swapchain_imageviews.reserve( swapchain_images.size() );
		for( const vk::Image& image : swapchain_images )
		{
			vk::ImageViewCreateInfo image_view_create_info
			{
				.image = image,
				.viewType = vk::ImageViewType::e2D,
				.format = swapchain_create_info.imageFormat,

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
			const vk::ImageView image_view = get_vk_result( rv_image_view, "failed to create image view from swapchain image" );

			swapchain_imageviews.push_back( image_view );
		}

		const Swapchain swapchain
		{
			.vkSwapchain = vk_swapchain,
			.images = swapchain_images,
			.imageViews = swapchain_imageviews,
			.imageExtent = swapchain_create_info.imageExtent,
			.imageFormat = swapchain_create_info.imageFormat,
			.imageColorSpace = swapchain_create_info.imageColorSpace,
			.presentMode = swapchain_create_info.presentMode,
		};

		return swapchain;
	}


	void destroy_swapchain( const vk::Device& device, Swapchain& swapchain )
	{
		for( const vk::ImageView& image_view : swapchain.imageViews )
		{
			device.destroy( image_view );
		}

		swapchain.imageViews.clear();
		swapchain.images.clear();

		device.destroy( swapchain.vkSwapchain );
	}
}