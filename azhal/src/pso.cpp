#include "azpch.h"
#include "pso.h"

namespace azhal
{
	PSO::PSO( const vk::Device& device, const PSOCreateInfo& pso_create_info )
	{
		const auto shader_module_create_fn = [&device, this]( const AnsiChar* file_path ) -> const vk::ShaderModule
		{
			const ByteBufferDynamic& shader_code = LoadBinaryBlob( file_path );
			const vk::ShaderModuleCreateInfo shader_create_info
			{
				.codeSize = VK_SIZE_CAST( shader_code.size() ),
				.pCode = reinterpret_cast< const Uint32* >( shader_code.data() )
			};
			const vk::ResultValue rv_shader_module = device.createShaderModule( shader_create_info );
			return ( CheckVkResultValue( rv_shader_module, "failed to create shader module" ) );
		};

		const vk::ShaderModule& vertex_shader_module = shader_module_create_fn( pso_create_info.vertexShader );
		const vk::ShaderModule& fragment_shader_module = shader_module_create_fn( pso_create_info.fragmentShader );

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

		device.destroy( vertex_shader_module );
		device.destroy( fragment_shader_module );
	}
}