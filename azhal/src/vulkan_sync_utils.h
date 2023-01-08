#pragma once

namespace gdevice
{
	enum AccessTypeBits : Uint32;

	void insert_image_pipeline_barrier( vk::CommandBuffer cmd_buffer, vk::Image image,
		vk::ImageLayout src_layout, AccessTypeBits src_access_type_mask,
		vk::ImageLayout dst_layout, AccessTypeBits dst_access_type_mask,
		Uint32 base_mip_level = 0, Uint32 mip_count = 1,
		Uint32 base_array_layer = 0, Uint32 layer_count = 1
	);
}