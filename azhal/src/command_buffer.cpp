#include "azpch.h"
#include "command_buffer.h"

#include "enums.h"

namespace
{
	vk::CommandPool s_computeCommandPool;
	vk::CommandPool s_graphicsCommandPool;
	vk::CommandPool s_transferCommandPool;
	vk::CommandPool s_presentCommandPool;
}

namespace
{
	vk::CommandPool create_command_pool( const vk::Device& device, Uint32 queue_family_index )
	{
		vk::CommandPoolCreateInfo cmd_pool_create_info
		{
			.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			.queueFamilyIndex = queue_family_index
		};

		const vk::ResultValue rv_command_pool = device.createCommandPool( cmd_pool_create_info );
		return ( gdevice::get_vk_result( rv_command_pool, "failed to create command pool" ) );
	}
}

namespace gdevice
{
	void init_command_pools( const vk::Device& device, const CommandPoolsInitParams& cmd_pool_init_params )
	{
		s_computeCommandPool = create_command_pool( device, cmd_pool_init_params.computeQueueFamilyIndex );
		s_graphicsCommandPool = create_command_pool( device, cmd_pool_init_params.graphicsQueueFamilyIndex );
		s_presentCommandPool = create_command_pool( device, cmd_pool_init_params.presentQueueFamilyIndex );
		s_transferCommandPool = create_command_pool( device, cmd_pool_init_params.transferQueueFamilyIndex );
	}


	void destroy_command_pools( const vk::Device& device )
	{
		device.destroy( s_computeCommandPool );
		device.destroy( s_graphicsCommandPool );
		device.destroy( s_presentCommandPool );
		device.destroy( s_transferCommandPool );
	}

	//TODO: this is only for 1 command buffer now, change it to be for multiple
	vk::CommandBuffer allocate_command_buffer( const vk::Device& device, QueueType queue_type, vk::CommandBufferLevel cmd_buffer_level )
	{
		vk::CommandBufferAllocateInfo cmd_buffer_alloc_info
		{
			.level = cmd_buffer_level,
			.commandBufferCount = 1
		};

		switch( queue_type )
		{
		case QueueType::eGraphics:
			cmd_buffer_alloc_info.commandPool = s_graphicsCommandPool;
			break;
		case QueueType::eCompute:
			cmd_buffer_alloc_info.commandPool = s_computeCommandPool;
			break;
		case QueueType::ePresent:
			cmd_buffer_alloc_info.commandPool = s_presentCommandPool;
			break;
		case QueueType::eTransfer:
			cmd_buffer_alloc_info.commandPool = s_transferCommandPool;
			break;
		default:
			AZHAL_LOG_CRITICAL( "Invalid queue type when allocating command buffer" );
			AZHAL_DEBUG_BREAK();
			break;
		}

		const vk::ResultValue rv_command_buffer = device.allocateCommandBuffers( cmd_buffer_alloc_info );
		const std::vector<vk::CommandBuffer>& cmd_buffers = get_vk_result( rv_command_buffer, "failed to allocate command buffer" );

		return cmd_buffers[ 0 ];
	}

	void free_command_buffer( vk::Device device, QueueType queue_type, vk::CommandBuffer cmd_buffer )
	{
		switch( queue_type )
		{
		case gdevice::QueueType::eGraphics:
			device.freeCommandBuffers( s_graphicsCommandPool, cmd_buffer );
			break;
		case gdevice::QueueType::eCompute:
			device.freeCommandBuffers( s_computeCommandPool, cmd_buffer );
			break;
		case gdevice::QueueType::ePresent:
			device.freeCommandBuffers( s_presentCommandPool, cmd_buffer );
			break;
		case gdevice::QueueType::eTransfer:
			device.freeCommandBuffers( s_transferCommandPool, cmd_buffer );
			break;
		case gdevice::QueueType::eInvalid:
		default:
			break;
		}
	}
}
