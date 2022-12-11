#pragma once
#include <vulkan/vulkan.hpp>

namespace azhal
{
	class Window;

	struct RendererCreateInfo
	{
	public:
		const WindowPtr& pWindow = nullptr;
		Bool IsValidationLayersEnabled = false;
		Bool IsGpuAssistedValidationEnabled = false;
		vk::DebugUtilsMessageSeverityFlagBitsEXT DebugMessageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;
	};

	class Renderer
	{
	public:
		explicit Renderer( const RendererCreateInfo& renderer_create_info );
		~Renderer();

		Renderer() = delete;
		Renderer( const Renderer& ) = delete;

		static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback( vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
			vk::DebugUtilsMessageTypeFlagBitsEXT message_type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData );

	private:
		void CreateInstance();
		void CreateDevice();

		std::vector<const char*> GetRequiredExtensions() const;
		std::vector<const char*> GetValidationLayers() const;
		vk::PhysicalDevice GetSuitablePhysicalDevice() const;
		vk::DebugUtilsMessengerCreateInfoEXT GetDebugUtilsMessengerCreateInfo( const PFN_vkDebugUtilsMessengerCallbackEXT& debug_callback_fn ) const;

	private:
		Bool m_validationLayersEnabled;
		Bool m_gpuAssistedValidationEnabled;
		vk::DebugUtilsMessageSeverityFlagBitsEXT m_debugMessageSeverity;

		vk::Instance m_instance;
		vk::DispatchLoaderDynamic m_DynamicDispatchInstance;
		vk::DebugUtilsMessengerEXT m_debugMessenger;
	};

	using RendererPtr = std::unique_ptr<Renderer>;
}