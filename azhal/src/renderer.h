#pragma once
#include <vulkan/vulkan.hpp>

#include "renderDevice.h"

namespace azhal
{
	class Window;

	struct RendererCreateInfo
	{
	public:
		const Window& window;
		Bool IsValidationLayersEnabled = false;
		Bool IsGpuAssistedValidationEnabled = false;
		vk::DebugUtilsMessageSeverityFlagBitsEXT DebugMessageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;
	};

	class Renderer
	{
	public:
		explicit Renderer( const RendererCreateInfo& renderer_create_info );
		void destroy();

		Renderer() = delete;
		Renderer( const Renderer& ) = delete;

		static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback( vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
			vk::DebugUtilsMessageTypeFlagBitsEXT message_type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData );


	private:
		vk::Instance m_instance;
		vk::DispatchLoaderDynamic m_instanceDynamicDispatchLoader;
		vk::DebugUtilsMessengerEXT m_debugMessenger;

		vk::SurfaceKHR m_surface;
		vk::Device m_device;
		Swapchain m_swapchain;
	};

	using RendererPtr = std::unique_ptr<Renderer>;
}