#pragma once

namespace gdevice
{
	enum class QueueType : Uint32;

	struct CommandPoolsInitParams
	{
		const Uint32 computeQueueFamilyIndex = UINT32_MAX;
		const Uint32 graphicsQueueFamilyIndex = UINT32_MAX;
		const Uint32 presentQueueFamilyIndex = UINT32_MAX;
		const Uint32 transferQueueFamilyIndex = UINT32_MAX;
	};

	void init_command_pools( const vk::Device& device, const CommandPoolsInitParams& cmd_pool_init_params );
	void destroy_command_pools( const vk::Device& device );

	vk::CommandBuffer allocate_command_buffer( const vk::Device& device, QueueType queue_type, vk::CommandBufferLevel cmd_buffer_level = vk::CommandBufferLevel::ePrimary );
	void free_command_buffer( vk::Device device, QueueType queue_type, vk::CommandBuffer cmd_buffer );

}