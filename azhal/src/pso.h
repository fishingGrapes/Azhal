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
	private:
	};
}