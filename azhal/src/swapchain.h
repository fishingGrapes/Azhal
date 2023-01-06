#pragma once

namespace azhal
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
}