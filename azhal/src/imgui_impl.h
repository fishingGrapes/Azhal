#pragma once

namespace gdevice
{
	struct ImguiInitParams
	{
		void* pWindow = nullptr;
		const vk::Instance instance;
		const vk::PhysicalDevice physicalDevice;
		const vk::Device device;
		Uint32 graphicsQueueFamilyIndex = UINT32_MAX;
		const vk::Queue graphicsQueue;
		const vk::Format swapchainImageFormat = vk::Format::eUndefined;
		Uint32 swapchainImageCount = 0;
	};

	void init_imgui( const ImguiInitParams& imgui_init_params );
	void begin_imgui();
	void end_imgui( vk::CommandBuffer cmd_buffer );
	void shutdown_imgui( const vk::Device device );
}