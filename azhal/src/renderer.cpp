#include "azpch.h"
#include "window.h"
#include "renderer.h"
#include "enums.h"

#include <GLFW/glfw3.h>

#include "vulkan_init_helper.h"

namespace azhal
{
	Renderer::Renderer( const RendererCreateInfo& renderer_create_info )
	{
		m_instanceDynamicDispatchLoader = vk::DispatchLoaderDynamic( m_instance, vkGetInstanceProcAddr );

#ifdef AZHAL_ENABLE_LOGGING
		m_debugMessenger = create_debug_messenger()
#endif
	}

	void Renderer::destroy()
	{
		m_device.destroy( m_swapchain );
		m_device.destroy();

		m_instance.destroy( m_surface );
		AZHAL_LOG_WARN( "vulkan surface destroyed" );

#ifdef AZHAL_ENABLE_LOGGING
		m_instance.destroyDebugUtilsMessengerEXT( m_debugMessenger, VK_NULL_HANDLE, m_instanceDynamicDispatchLoader );
#endif
		m_instance.destroy();
		AZHAL_LOG_WARN( "vulkan instance destroyed" );
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	VKAPI_ATTR vk::Bool32 VKAPI_CALL Renderer::DebugCallback( vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity, vk::DebugUtilsMessageTypeFlagBitsEXT message_type,
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData )
	{
		String message_type_name = "";
		switch( message_type )
		{
		case vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral:
			message_type_name = "[vk_general]";
			break;
		case vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation:
			message_type_name = "[vk_validation]";
			break;
		case vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance:
			message_type_name = "[vk_performance]";
			break;
		case vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding:
			message_type_name = "[vk_deviceAddressBinding]";
			break;
		default:
			AZHAL_LOG_CRITICAL( "Invalid vk::DebugUtilsMessageTypeFlagBitsEXT flag bit: {0}", message_type );
			AZHAL_DEBUG_BREAK();
			break;
		}

		switch( message_severity )
		{
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
			AZHAL_LOG_TRACE( "{0} {1}", message_type_name, pCallbackData->pMessage );
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
			AZHAL_LOG_INFO( "{0} {1}", message_type_name, pCallbackData->pMessage );
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
			AZHAL_LOG_WARN( "{0} {1}", message_type_name, pCallbackData->pMessage );
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
			AZHAL_LOG_ERROR( "{0} {1}", message_type_name, pCallbackData->pMessage );
			break;
		default:
			AZHAL_LOG_CRITICAL( "Invalid vk::DebugUtilsMessageSeverityFlagBitsEXT flag bit: {0}", message_severity );
			AZHAL_DEBUG_BREAK();
			break;
		}

#ifdef VK_DEADLY_VALIDATION
		return VK_TRUE;
#else
		return VK_FALSE;
#endif
	}

}