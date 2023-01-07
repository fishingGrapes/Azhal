#pragma once
#include <vulkan/vulkan.hpp>

namespace gdevice
{
	struct PSOCreationParams
	{
		const AnsiChar* pVertexShader;
		const AnsiChar* pFragmentShader;
		Bool isDynamicRendering = VK_FALSE;
		const std::vector<vk::Format> colorAttachmentFormats;
	};

	struct PSO
	{
		vk::PipelineLayout pipelineLayout;
		vk::Pipeline vkPipelineObject;
	};

	PSO create_pso( const vk::Device& device, const PSOCreationParams& pso_creation_params );
	void destroy_pso( const vk::Device& device, PSO& pso );

}