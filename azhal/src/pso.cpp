#include "azpch.h"
#include "pso.h"

namespace gdevice
{
	PSO create_pso( const vk::Device device, const PSOCreationParams& pso_creation_params )
	{
		const auto shader_module_create_fn = [device]( const AnsiChar* file_path ) -> const vk::ShaderModule
		{
			const ByteBufferDynamic shader_code = LoadBinaryBlob( file_path );
			const vk::ShaderModuleCreateInfo shader_create_info
			{
				.codeSize = VK_SIZE_CAST( shader_code.size() ),
				.pCode = reinterpret_cast< const Uint32* >( shader_code.data() )
			};
			const vk::ResultValue rv_shader_module = device.createShaderModule( shader_create_info );
			return ( get_vk_result( rv_shader_module, "failed to create shader module" ) );
		};

		const vk::ShaderModule vertex_shader_module = shader_module_create_fn( pso_creation_params.pVertexShader );
		const vk::ShaderModule fragment_shader_module = shader_module_create_fn( pso_creation_params.pFragmentShader );

		const vk::PipelineShaderStageCreateInfo vertex_shader_stage_create_info
		{
			.stage = vk::ShaderStageFlagBits::eVertex,
			.module = vertex_shader_module,
			.pName = "main"
		};
		const vk::PipelineShaderStageCreateInfo fragment_shader_stage_create_info
		{
			.stage = vk::ShaderStageFlagBits::eFragment,
			.module = fragment_shader_module,
			.pName = "main"
		};
		const vk::PipelineShaderStageCreateInfo shader_stages[ 2 ] =
		{
			vertex_shader_stage_create_info,
			fragment_shader_stage_create_info
		};

		const vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info
		{
			.vertexBindingDescriptionCount = 0,
			.pVertexBindingDescriptions = VK_NULL_HANDLE,
			.vertexAttributeDescriptionCount = 0,
			.pVertexAttributeDescriptions = VK_NULL_HANDLE
		};

		const vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_create_info
		{
			.topology = vk::PrimitiveTopology::eTriangleList,
			.primitiveRestartEnable = VK_FALSE
		};

		const vk::PipelineViewportStateCreateInfo viewport_state_create_info
		{
			.viewportCount = 1,
			.scissorCount = 1
		};

		const vk::PipelineRasterizationStateCreateInfo raster_state_create_info
		{
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = vk::PolygonMode::eFill,
			.cullMode = vk::CullModeFlagBits::eBack,
			.frontFace = vk::FrontFace::eClockwise,
			.depthBiasEnable = VK_FALSE,
			.lineWidth = 1.0f
		};

		const vk::PipelineMultisampleStateCreateInfo multisample_state_create_info
		{
			.rasterizationSamples = vk::SampleCountFlagBits::e1,
			.sampleShadingEnable = VK_FALSE
		};

		const vk::PipelineColorBlendAttachmentState color_blend_attch_state
		{
			.blendEnable = VK_TRUE,
			.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
			.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
			.colorBlendOp = vk::BlendOp::eAdd,
			.srcAlphaBlendFactor = vk::BlendFactor::eOne,
			.dstAlphaBlendFactor = vk::BlendFactor::eZero,
			.alphaBlendOp = vk::BlendOp::eAdd,
			.colorWriteMask = ( vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
								vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA )
		};
		const vk::PipelineColorBlendStateCreateInfo color_blend_state_create_info
		{
			.logicOpEnable = VK_FALSE,
			.logicOp = vk::LogicOp::eCopy,
			.attachmentCount = 1,
			.pAttachments = &color_blend_attch_state,
			.blendConstants = std::array<Float, 4>{0, 0, 0, 0}
		};

		const std::vector<vk::DynamicState> dynamic_states
		{
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};
		const vk::PipelineDynamicStateCreateInfo dynamic_state_create_info
		{
			.dynamicStateCount = VK_SIZE_CAST( dynamic_states.size() ),
			.pDynamicStates = dynamic_states.data()
		};

		const vk::PipelineLayoutCreateInfo pipeline_layout_create_info
		{
			.setLayoutCount = 0,
			.pSetLayouts = VK_NULL_HANDLE,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = VK_NULL_HANDLE
		};
		const vk::ResultValue rv_pipleline_layout = device.createPipelineLayout( pipeline_layout_create_info );
		const vk::PipelineLayout pipeline_layout = get_vk_result( rv_pipleline_layout, "failed to create pipeline layout" );

		const vk::PipelineRenderingCreateInfo pipeline_rendering_create_info
		{
			.colorAttachmentCount = VK_SIZE_CAST( pso_creation_params.colorAttachmentFormats.size() ),
			.pColorAttachmentFormats = pso_creation_params.colorAttachmentFormats.data()
		};

		const vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info
		{
			.stageCount = 2,
			.pStages = shader_stages,
			.pVertexInputState = &vertex_input_state_create_info,
			.pInputAssemblyState = &input_assembly_state_create_info,
			.pViewportState = &viewport_state_create_info,
			.pRasterizationState = &raster_state_create_info,
			.pMultisampleState = &multisample_state_create_info,
			.pDepthStencilState = VK_NULL_HANDLE,
			.pColorBlendState = &color_blend_state_create_info,
			.pDynamicState = &dynamic_state_create_info,
			.layout = pipeline_layout,
			.renderPass = VK_NULL_HANDLE
		};

		const vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> graphics_pipeline_creation_chain
		{
			graphics_pipeline_create_info,
			pipeline_rendering_create_info
		};

		const vk::ResultValue rv_graphics_pipeline = device.createGraphicsPipeline( VK_NULL_HANDLE, graphics_pipeline_creation_chain.get<vk::GraphicsPipelineCreateInfo>() );
		const vk::Pipeline vk_pipeline = get_vk_result( rv_graphics_pipeline, "failed to create graphics pipeline" );

		device.destroy( vertex_shader_module );
		device.destroy( fragment_shader_module );

		const PSO pso
		{
			.pipelineLayout = pipeline_layout,
			.vkPipelineObject = vk_pipeline
		};

		return pso;
	}

	void destroy_pso( const vk::Device device, PSO& pso )
	{
		device.destroy( pso.vkPipelineObject );
		device.destroy( pso.pipelineLayout );
	}

}