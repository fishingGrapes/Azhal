#pragma once
#include <vulkan/vulkan.hpp>

namespace azhal
{
	class Window;

	class Renderer
	{
	public:
		explicit Renderer( const WindowPtr& pWindow );
		~Renderer();

		Renderer() = delete;
		Renderer( const Renderer& ) = delete;

		//Bool Init();


		AZHAL_INLINE void EnableValdiationLayers()
		{
			m_validationLayersEnabled = true;
		}

		AZHAL_INLINE void EnableGpuAssistedValdiation()
		{
			m_gpuAssistedValidationEnabled = true;
		}

		AZHAL_INLINE void SetLogLevel( vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity )
		{
			m_debugMessageSeverity = message_severity;
		}

		static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback( vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
			vk::DebugUtilsMessageTypeFlagBitsEXT message_type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData );

	private:
		void CreateInstance();

		std::vector<const char*> GetRequiredExtensions() const;
		std::vector<const char*> GetValidationLayers() const;
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