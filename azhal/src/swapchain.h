#pragma once

namespace gdevice
{
	// TODO: change swapchain creation and deletion
	struct Swapchain : NonCopyable
	{
	public:

		explicit Swapchain( const vk::SwapchainKHR vk_swapchain, const std::vector<vk::Image>& images_, const std::vector<vk::ImageView>& image_views,
			const vk::Extent2D image_extent, const vk::Format image_format, const vk::ColorSpaceKHR image_color_space, const vk::PresentModeKHR present_mode )
			: vkSwapchain( vk_swapchain )
			, images( images_ )
			, imageViews( image_views )
			, imageExtent( image_extent )
			, imageFormat( image_format )
			, imageColorSpace( image_color_space )
			, presentMode( present_mode )
		{
		}

		Swapchain( Swapchain&& other )
			: vkSwapchain( other.vkSwapchain )
			, images( std::move( other.images ) )
			, imageViews( std::move( other.imageViews ) )
			, imageExtent( other.imageExtent )
			, imageFormat( other.imageFormat )
			, imageColorSpace( other.imageColorSpace )
			, presentMode( other.presentMode )
		{

			other.vkSwapchain = VK_NULL_HANDLE;
			other.images.clear();
			other.imageViews.clear();
		}

		Swapchain& operator=( Swapchain&& other )
		{
			if( this == &other )
			{
				return *this;
			}

			AZHAL_FATAL_ASSERT( vkSwapchain, "trying to move to a non-empty swapchain object" );
			AZHAL_FATAL_ASSERT( images.size() == 0, "trying to move to a non-empty swapchain object" );
			AZHAL_FATAL_ASSERT( imageViews.size() == 0, "trying to move to a non-empty swapchain object" );

			vkSwapchain = other.vkSwapchain;
			images = std::move( other.images );
			imageViews = std::move( other.imageViews );
			imageExtent = other.imageExtent;
			imageFormat = other.imageFormat;
			imageColorSpace = other.imageColorSpace;
			presentMode = other.presentMode;

			other.vkSwapchain = VK_NULL_HANDLE;
			other.images.clear();
			other.imageViews.clear();

			return *this;
		}

		Swapchain() = delete;

		//----------------------------------------------------------------//

		vk::SwapchainKHR vkSwapchain = VK_NULL_HANDLE;

		std::vector<vk::Image> images;
		std::vector<vk::ImageView> imageViews;

		vk::Extent2D imageExtent;
		vk::Format imageFormat;
		vk::ColorSpaceKHR imageColorSpace;
		vk::PresentModeKHR presentMode;
	};


	Swapchain create_swapchain( const vk::PhysicalDevice physical_device, const vk::Device device, const vk::SurfaceKHR surface, const vk::Extent2D& desired_extent );

	void destroy_swapchain( const vk::Device device, Swapchain& swapchain );

	Swapchain recreate_swapchain( const vk::PhysicalDevice physical_device, const vk::Device device, const vk::SurfaceKHR surface, const vk::Extent2D& desired_extent, Swapchain& old_swapchain );
}