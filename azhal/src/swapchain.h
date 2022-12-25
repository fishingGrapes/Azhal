#pragma once
#include <vulkan/vulkan.hpp>

namespace azhal
{
	class Swapchain
	{
	public:
		Swapchain()
			: m_format( vk::Format::eUndefined )
			, m_presentMode( vk::PresentModeKHR::eFifo )
		{
		}
		Swapchain( const vk::PhysicalDevice& physical_device, const vk::SurfaceKHR& surface );

	private:

		[[nodiscard( "azhal::Swapchain::ChooseFormat" )]]
		vk::Format ChooseFormat( const std::vector<vk::SurfaceFormatKHR>& available_formats );
		[[nodiscard( "azhal::Swapchain::ChoosePresentMode" )]]
		vk::PresentModeKHR ChoosePresentMode( const std::vector<vk::PresentModeKHR>& available_present_modes );
		[[nodiscard( "azhal::Swapchain::ChooseExtent" )]]
		vk::Extent2D ChooseExtent( const vk::SurfaceCapabilitiesKHR& surface_capabilties );

		vk::SwapchainKHR m_swapchain;
		std::vector<vk::Image> m_images;
		vk::Format m_format;
		vk::PresentModeKHR m_presentMode;
		vk::Extent2D m_extent;
	};
}