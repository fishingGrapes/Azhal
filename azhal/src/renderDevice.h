#pragma once
#include <vulkan/vulkan.hpp>

#include "window.h"
#include "gpuQueue.h"
#include "swapchain.h"
#include "pso.h"

namespace azhal
{
	class RenderDevice
	{
	public:
		RenderDevice() = default;
		RenderDevice( const vk::Instance& instance, const vk::SurfaceKHR& surface, const vk::DispatchLoaderDynamic& instance_ddl );

		AZHAL_INLINE void destroy()
		{
			m_device.destroy();
			AZHAL_LOG_WARN( "vulkan device destroyed" );
		}

		template <typename T>
		AZHAL_INLINE void destroy( T& t )
		{
			m_device.destroy( t );
		}

		template <>
		AZHAL_INLINE void destroy( Swapchain& swap_chain )
		{
			swap_chain.Destroy( m_device );
		}

		template <>
		AZHAL_INLINE void destroy( PSO& pso )
		{
			pso.Destroy( m_device );
		}

		AZHAL_INLINE const vk::Device& Get() const
		{
			return m_device;
		}

		AZHAL_INLINE const vk::PhysicalDevice& GetPhysicalDevice() const
		{
			return m_physicalDevice;
		}

		[[nodiscard( "azhal::RenderDevice::CreateSwapchain" )]]
		Swapchain CreateSwapchain( const WindowPtr& pWindow, const vk::SurfaceKHR& surface );

		[[nodiscard( "azhal::RenderDevice::CreatePSO" )]]
		PSO CreatePSO( const PSOCreateInfo& pso_create_info );

	private:
		[[nodiscard( "azhal::RenderDevice::GetSuitablePhysicalDevice" )]]
		vk::PhysicalDevice GetSuitablePhysicalDevice( const vk::Instance& instance ) const;
		[[nodiscard( "azhal::RenderDevice::GetRequiredDeviceExtensions" )]]
		const std::vector<const AnsiChar*> GetRequiredDeviceExtensions() const;


		vk::PhysicalDevice m_physicalDevice;
		vk::Device m_device;

		GpuQueue m_graphicsQueue;
		GpuQueue m_computeQueue;
		GpuQueue m_transferQueue;
		GpuQueue m_presentQueue;
	};
}