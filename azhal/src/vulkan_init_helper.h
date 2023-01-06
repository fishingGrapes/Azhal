#pragma once

namespace azhal
{
	struct VulkanInstanceCreationParams
	{
		Bool enableValidationLayers = false;
		vk::DebugUtilsMessageSeverityFlagBitsEXT debugMessageSeverity;
		const PFN_vkDebugUtilsMessengerCallbackEXT& debugCallbackFn;

		Bool enableGpuAssistedValidation = false;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	vk::Instance create_instance( const VulkanInstanceCreationParams& instance_creation_params );

	vk::DebugUtilsMessengerEXT create_debug_messenger( const vk::Instance& instance, vk::DebugUtilsMessageSeverityFlagBitsEXT debug_message_severity,
		const PFN_vkDebugUtilsMessengerCallbackEXT& debug_callback_fn, const vk::DispatchLoaderDynamic& dynamic_dispatch_loader );

	vk::SurfaceKHR create_vulkan_surface( const vk::Instance& instance, void* p_window );

	vk::PhysicalDevice get_suitable_physical_device( const vk::Instance& instance );

	vk::Device create_device( const vk::Instance& instance, const vk::PhysicalDevice& physical_device, const vk::SurfaceKHR& surface, const vk::DispatchLoaderDynamic& dynamic_dispatch_loader );


	////////////////////////////////////////////////////////////////////////////////////////////////////////////
}