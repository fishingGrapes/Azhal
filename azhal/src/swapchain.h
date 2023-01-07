#pragma once

namespace gdevice
{
	// TODO: change swapchain creation and deletion
	struct Swapchain
	{
		vk::SwapchainKHR vkSwapchain = VK_NULL_HANDLE;

		std::vector<vk::Image> images;
		std::vector<vk::ImageView> imageViews;

		vk::Extent2D imageExtent;
		vk::Format imageFormat;
		vk::ColorSpaceKHR imageColorSpace;
		vk::PresentModeKHR presentMode;
	};


	Swapchain create_swapchain( const vk::PhysicalDevice& physical_device, const vk::Device& device, const vk::SurfaceKHR& surface, vk::Extent2D desired_extent );

	void destroy_swapchain( const vk::Device& device, Swapchain& swapchain );
}