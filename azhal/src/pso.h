#pragma once
#include <vulkan/vulkan.hpp>

namespace azhal
{
	struct PSOCreateInfo
	{
	public:
		const AnsiChar* vertexShader;
		const AnsiChar* fragmentShader;
		Bool IsDynamicRendering = VK_FALSE;
		const std::vector<vk::Format> colorAttachmentFormats;
	};

	class PSO
	{
	public:
		PSO( const vk::Device& device, const PSOCreateInfo& pso_create_info );
		void Destroy( const vk::Device& device );

	private:
		vk::PipelineLayout m_pipelineLayout;
		vk::Pipeline m_pipeline;
	};
}