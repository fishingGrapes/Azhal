#include "azpch.h"
#include "swapchain.h"

#include "vulkanHelper.h"

namespace azhal
{
	Swapchain::Swapchain( const vk::PhysicalDevice& physical_device, const vk::SurfaceKHR& surface )
		: m_format( vk::Format::eUndefined )
		, m_presentMode( vk::PresentModeKHR::eFifo )
	{
		const vk::ResultValue surface_caps_rv = physical_device.getSurfaceCapabilitiesKHR( surface );
		const vk::SurfaceCapabilitiesKHR& surface_caps = CheckVkResultValue( surface_caps_rv, "failed to get surface capabilities" );

		const vk::ResultValue surface_formats_rv = physical_device.getSurfaceFormatsKHR( surface );
		const std::vector<vk::SurfaceFormatKHR>& surface_formats = CheckVkResultValue( surface_formats_rv, "failed to get surface formats" );

		const vk::ResultValue surface_present_modes_rv = physical_device.getSurfacePresentModesKHR( surface );
		const std::vector<vk::PresentModeKHR>& surface_present_modes = CheckVkResultValue( surface_present_modes_rv, "failed to get present modes" );

		AZHAL_FATAL_ASSERT( !surface_formats.empty() && !surface_present_modes.empty(), "swapchain support is not adequate" );

		m_format = ChooseFormat( surface_formats );
		m_presentMode = ChoosePresentMode( surface_present_modes );
		m_extent = ChooseExtent( surface_caps );

		// TODO: implement swapchain creation
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	vk::Format Swapchain::ChooseFormat( const std::vector<vk::SurfaceFormatKHR>& available_formats )
	{
		for( const vk::SurfaceFormatKHR& format : available_formats )
		{
			if( format.format == vk::Format::eR8G8B8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear )
				return vk::Format::eR8G8B8A8Srgb;

		}
		return available_formats[ 0 ].format;
	}

	vk::PresentModeKHR Swapchain::ChoosePresentMode( const std::vector<vk::PresentModeKHR>& available_present_modes )
	{
		for( const vk::PresentModeKHR& present_mode : available_present_modes )
		{
			if( present_mode == vk::PresentModeKHR::eMailbox )
				return vk::PresentModeKHR::eMailbox;

		}
		return vk::PresentModeKHR::eFifo;
	}

	vk::Extent2D Swapchain::ChooseExtent( const vk::SurfaceCapabilitiesKHR& surface_capabilties )
	{
		// TODO: implement choose extent
		return vk::Extent2D( 0, 0 );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
}