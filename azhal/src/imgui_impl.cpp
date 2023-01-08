#include "azpch.h"
#include "imgui_impl.h"

#include "command_buffer.h"
#include "enums.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

//https://frguthmann.github.io/posts/vulkan_imgui/

//TODO: fix validation error
//[ VUID-vkDestroyDevice-device-00378 ] Object 0: handle = 0x164b7712fd0, type = VK_OBJECT_TYPE_DEVICE; Object 1: handle = 0x27d60e0000000019, type = VK_OBJECT_TYPE_RENDER_PASS; 
//MessageID = 0x71500fba | OBJ ERROR : For VkDevice 0x164b7712fd0[], VkRenderPass 0x27d60e0000000019[] has not been destroyed.
//The Vulkan spec states: All child objects created on device must have been destroyed prior to destroying device 
//(https://vulkan.lunarg.com/doc/view/1.3.236.0/windows/1.3-extensions/vkspec.html#VUID-vkDestroyDevice-device-00378)


namespace
{
	vk::DescriptorPool s_descriptorPool;
	vk::RenderPass s_renderpass;
}

namespace
{

	vk::DescriptorPool create_descriptor_pool( const vk::Device device )
	{
		static const vk::DescriptorPoolSize pool_sizes[] =
		{
			{ vk::DescriptorType::eSampler, 1000 },
			{ vk::DescriptorType::eCombinedImageSampler, 1000 },
			{ vk::DescriptorType::eSampledImage, 1000 },
			{ vk::DescriptorType::eStorageImage, 1000 },
			{ vk::DescriptorType::eUniformTexelBuffer, 1000 },
			{ vk::DescriptorType::eStorageTexelBuffer, 1000 },
			{ vk::DescriptorType::eUniformBuffer, 1000 },
			{ vk::DescriptorType::eStorageBuffer, 1000 },
			{ vk::DescriptorType::eUniformBufferDynamic, 1000 },
			{ vk::DescriptorType::eStorageBufferDynamic, 1000 },
			{ vk::DescriptorType::eInputAttachment, 1000 }
		};

		const vk::DescriptorPoolCreateInfo pool_info
		{
			.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
			.maxSets = 1000,
			.poolSizeCount = VK_SIZE_CAST( std::size( pool_sizes ) ),
			.pPoolSizes = pool_sizes
		};

		const vk::ResultValue rv_descriptor_pool = device.createDescriptorPool( pool_info );
		return gdevice::get_vk_result( rv_descriptor_pool, "failed to create descriptor pool for imgui" );
	}

	vk::RenderPass build_renderpass( const vk::Device device, vk::Format swapchainImageFormat )
	{
		VkAttachmentDescription attachment = {};
		attachment.format = static_cast< VkFormat >( swapchainImageFormat );
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment = {};
		color_attachment.attachment = 0;
		color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;  // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPass renderpass = VK_NULL_HANDLE;

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = 1;
		info.pAttachments = &attachment;
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = 1;
		info.pDependencies = &dependency;
		if( vkCreateRenderPass( device, &info, nullptr, &renderpass ) != VK_SUCCESS ) {
			throw std::runtime_error( "Could not create Dear ImGui's render pass" );
		}
		
		return renderpass;
	}
}

namespace gdevice
{
	void init_imgui( const ImguiInitParams& init_params )
	{

		// 1: initialize imgui library

		//this initializes the core structures of imgui
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		//this initializes imgui for GLFW
		ImGui_ImplGlfw_InitForVulkan( static_cast< GLFWwindow* >( init_params.pWindow ), true );


		s_descriptorPool = create_descriptor_pool( init_params.device );

		ImGui_ImplVulkan_InitInfo init_info
		{
			.Instance = init_params.instance,
			.PhysicalDevice = init_params.physicalDevice,
			.Device = init_params.device,
			.QueueFamily = init_params.graphicsQueueFamilyIndex,
			.Queue = init_params.graphicsQueue,
			.DescriptorPool = s_descriptorPool,
			.MinImageCount = init_params.swapchainImageCount,
			.ImageCount = init_params.swapchainImageCount,
			.MSAASamples = VK_SAMPLE_COUNT_1_BIT
		};

		s_renderpass = build_renderpass( init_params.device, init_params.swapchainImageFormat );
		ImGui_ImplVulkan_Init( &init_info, s_renderpass );

		vk::CommandBuffer font_upload_cmd_buffer = allocate_command_buffer( init_params.device, QueueType::eGraphics );
		font_upload_cmd_buffer.reset();

		const vk::CommandBufferBeginInfo cmd_buffer_begin_info {};
		const vk::Result res_cmd_buffer_begin = font_upload_cmd_buffer.begin( cmd_buffer_begin_info );
		vk::resultCheck( res_cmd_buffer_begin, "failed to begin imgui font upload command buffer" );

		ImGui_ImplVulkan_CreateFontsTexture( font_upload_cmd_buffer );

		const vk::Result res_cmd_buffer_end = font_upload_cmd_buffer.end();
		vk::resultCheck( res_cmd_buffer_end, "failed to end imgui font upload command buffer" );

		const vk::FenceCreateInfo fence_create_info { .flags = vk::FenceCreateFlagBits::eSignaled };
		const vk::ResultValue rv_font_upload_fence = init_params.device.createFence( fence_create_info );
		const vk::Fence  font_upload_fence = get_vk_result( rv_font_upload_fence, "failed to create imgui font upload fence" );

		const vk::Result res_rest_fences = init_params.device.resetFences( font_upload_fence );
		vk::resultCheck( res_rest_fences, "failed to reset imgui font upload fence" );

		const vk::SubmitInfo submit_info
		{
			.waitSemaphoreCount = 0,
			.pWaitSemaphores = VK_NULL_HANDLE,
			.pWaitDstStageMask = VK_NULL_HANDLE,
			.commandBufferCount = 1,
			.pCommandBuffers = &font_upload_cmd_buffer,
			.signalSemaphoreCount = 0,
			.pSignalSemaphores = VK_NULL_HANDLE
		};

		const vk::Result res_submit_gfx_cmd_buffer = init_params.graphicsQueue.submit( submit_info, font_upload_fence );
		vk::resultCheck( res_submit_gfx_cmd_buffer, "failed to submit gfx command buffer" );

		const vk::Result res_wait_for_fences = init_params.device.waitForFences( font_upload_fence, VK_TRUE, UINT64_MAX );
		vk::resultCheck( res_wait_for_fences, "failed to wait for imgui font upload fence" );

		ImGui_ImplVulkan_DestroyFontUploadObjects();

		free_command_buffer( init_params.device, QueueType::eGraphics, font_upload_cmd_buffer );
		init_params.device.destroy( font_upload_fence );
	}


	void begin_imgui()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow();

		ImGui::Render();
	}

	void end_imgui( vk::CommandBuffer cmd_buffer )
	{
		ImGui_ImplVulkan_RenderDrawData( ImGui::GetDrawData(), cmd_buffer );
	}

	void shutdown_imgui( const vk::Device device )
	{
		device.destroy( s_renderpass );
		device.destroy( s_descriptorPool );
		ImGui_ImplVulkan_Shutdown();
	}
}