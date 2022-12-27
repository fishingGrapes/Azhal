#pragma once
#include <vulkan/vulkan.hpp>

namespace azhal
{
	struct PSOCreateInfo
	{
	public:
		const AnsiChar* vertexShader;
		const AnsiChar* fragmentShader;
	};

	class PSO
	{
	public:
		PSO( const vk::Device& device, const PSOCreateInfo& pso_create_info );

		AZHAL_INLINE void Destroy( const vk::Device& device )
		{
			device.destroy( m_pipelineLayout );
		}

	private:
		vk::PipelineLayout m_pipelineLayout;
	};
}