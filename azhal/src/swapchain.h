#pragma once
#include <vulkan/vulkan.hpp>
#include "window.h"

namespace azhal
{
	class Swapchain
	{
	public:
		Swapchain()
			: m_format( vk::Format::eUndefined )
			, m_colorSpace( vk::ColorSpaceKHR::eSrgbNonlinear )
			, m_presentMode( vk::PresentModeKHR::eFifo )
		{
		}
		Swapchain( const WindowPtr& pWindow, const vk::PhysicalDevice& physical_device, const vk::Device& device, const vk::SurfaceKHR& surface );

		AZHAL_INLINE vk::Format GetFormat() const
		{
			return m_format;
		}

		void Destroy( const vk::Device& device );

	private:
		vk::SwapchainKHR m_swapchain;
		std::vector<vk::Image> m_images;
		std::vector<vk::ImageView> m_imageViews;

		vk::Format m_format;
		vk::ColorSpaceKHR m_colorSpace;
		vk::PresentModeKHR m_presentMode;
		vk::Extent2D m_extent;
	};
}