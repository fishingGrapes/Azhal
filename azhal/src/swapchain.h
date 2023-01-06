#pragma once
#include <vulkan/vulkan.hpp>
#include "window.h"

namespace azhal
{
	class Swapchain
	{
	public:
		const vk::SwapchainKHR vkSwapchain;
		const std::vector<vk::Image> images;
		const std::vector<vk::ImageView> imageViews;

		const vk::Format imageFormat;
		const vk::ColorSpaceKHR imageColorSpace;
		const vk::PresentModeKHR presentMode;
		const vk::Extent2D imageExtent;
	};
}