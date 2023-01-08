#include "azpch.h"
#include "vulkan_sync_utils.h"

#include "enums.h"
//reading: https://gpuopen.com/learn/vulkan-barriers-explained/

namespace
{
	struct PipelineBarrierParams
	{
		vk::AccessFlags srcAccessMask = vk::AccessFlagBits::eNone;
		vk::PipelineStageFlags srcStageMask = vk::PipelineStageFlagBits::eTopOfPipe;
		vk::AccessFlags destAccessMask = vk::AccessFlagBits::eNone;
		vk::PipelineStageFlags dstStageMask = vk::PipelineStageFlagBits::eTopOfPipe;
		vk::ImageAspectFlags subResourceAspectMask;
	};

	PipelineBarrierParams get_pipeline_barrier_params( vk::ImageLayout src_layout, gdevice::AccessTypeBits src_access_type_mask, vk::ImageLayout dst_layout, gdevice::AccessTypeBits dst_access_type_mask )
	{
		using AccessTypeBits = gdevice::AccessTypeBits;
		PipelineBarrierParams barrier_params;

		if( src_layout == vk::ImageLayout::eUndefined && dst_layout == vk::ImageLayout::eColorAttachmentOptimal )
		{
			barrier_params.srcAccessMask = vk::AccessFlagBits::eNone;
			barrier_params.srcStageMask = vk::PipelineStageFlagBits::eTopOfPipe;
			barrier_params.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

			if( dst_access_type_mask & AccessTypeBits::eAccessTypeRead ) barrier_params.destAccessMask |= vk::AccessFlagBits::eColorAttachmentRead;
			if( dst_access_type_mask & AccessTypeBits::eAccessTypeWrite ) barrier_params.destAccessMask |= vk::AccessFlagBits::eColorAttachmentWrite;

			barrier_params.subResourceAspectMask = vk::ImageAspectFlagBits::eColor;
		}
		else if( src_layout == vk::ImageLayout::eColorAttachmentOptimal && dst_layout == vk::ImageLayout::ePresentSrcKHR )
		{
			barrier_params.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			barrier_params.dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
			barrier_params.destAccessMask = vk::AccessFlagBits::eNone;

			if( src_access_type_mask & AccessTypeBits::eAccessTypeRead ) barrier_params.srcAccessMask |= vk::AccessFlagBits::eColorAttachmentRead;
			if( src_access_type_mask & AccessTypeBits::eAccessTypeWrite ) barrier_params.srcAccessMask |= vk::AccessFlagBits::eColorAttachmentWrite;

			barrier_params.subResourceAspectMask = vk::ImageAspectFlagBits::eColor;
		}

		return barrier_params;
	}
}

namespace gdevice
{
	void insert_image_pipeline_barrier( vk::CommandBuffer cmd_buffer, vk::Image image,
		vk::ImageLayout src_layout, AccessTypeBits src_access_type_mask,
		vk::ImageLayout dst_layout, AccessTypeBits dst_access_type_mask,
		Uint32 base_mip_level, Uint32 mip_count, Uint32 base_array_layer, Uint32 layer_count )
	{

		const PipelineBarrierParams barrier_params = get_pipeline_barrier_params( src_layout, src_access_type_mask, dst_layout, dst_access_type_mask );

		const vk::ImageMemoryBarrier image_mem_barrier
		{
			.srcAccessMask = barrier_params.srcAccessMask,
			.dstAccessMask = barrier_params.destAccessMask,
			.oldLayout = src_layout,
			.newLayout = dst_layout,
			.image = image,

			.subresourceRange = {
									.aspectMask = barrier_params.subResourceAspectMask,
									.baseMipLevel = base_mip_level,
									.levelCount = mip_count,
									.baseArrayLayer = base_array_layer,
									.layerCount = layer_count
								}
		};

		cmd_buffer.pipelineBarrier( barrier_params.srcStageMask, barrier_params.dstStageMask, {}, {}, {}, image_mem_barrier );
	}
}