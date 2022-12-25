#pragma once
#include <vulkan/vulkan.hpp>
#include "gpuQueue.h"

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
		void CreateSurface( const WindowPtr& pWindow );
		void Destroy();

		[[nodiscard( "azhal::Renderer::GetRequiredInstanceExtensions" )]]
		std::vector<const AnsiChar*> GetRequiredInstanceExtensions() const;
		[[nodiscard( "azhal::Renderer::GetValidationLayers" )]]
		std::vector<const AnsiChar*> GetValidationLayers() const;
		[[nodiscard( "azhal::Renderer::GetEnabledValidationFeatures" )]]
		std::vector<vk::ValidationFeatureEnableEXT> GetEnabledValidationFeatures() const;
		[[nodiscard( "azhal::Renderer::GetSuitablePhysicalDevice" )]]
		vk::PhysicalDevice GetSuitablePhysicalDevice() const;
		[[nodiscard( "azhal::Renderer::GetDebugUtilsMessengerCreateInfo" )]]
		vk::DebugUtilsMessengerCreateInfoEXT GetDebugUtilsMessengerCreateInfo( const PFN_vkDebugUtilsMessengerCallbackEXT& debug_callback_fn ) const;
		[[nodiscard( "azhal::Renderer::GetRequiredDeviceExtensions" )]]
		std::vector<const AnsiChar*> GetRequiredDeviceExtensions() const;

	private:
		Bool m_validationLayersEnabled;
		Bool m_gpuAssistedValidationEnabled;
		vk::DebugUtilsMessageSeverityFlagBitsEXT m_debugMessageSeverity;

		vk::Instance m_instance;
		vk::DispatchLoaderDynamic m_DynamicDispatchInstance;
		vk::DebugUtilsMessengerEXT m_debugMessenger;

		vk::Device m_device;

		GpuQueue m_graphicsQueue;
		GpuQueue m_computeQueue;
		GpuQueue m_transferQueue;
		GpuQueue m_presentQueue;

		vk::SurfaceKHR m_surface;

	};

	using RendererPtr = std::unique_ptr<Renderer>;
}