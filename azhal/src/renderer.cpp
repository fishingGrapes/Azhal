#include "azpch.h"
#include "window.h"
#include "renderer.h"
#include "vulkanHelper.h"

#include <GLFW/glfw3.h>

namespace azhal
{
	Renderer::Renderer( const WindowPtr& pWindow )
		: m_validationLayersEnabled( false )
		, m_gpuAssistedValidationEnabled( false )
		, m_debugMessageSeverity( vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo )
	{
		CreateInstance();
	}

	Renderer::~Renderer()
	{
#ifdef AZHAL_ENABLE_LOGGING
		m_instance.destroyDebugUtilsMessengerEXT( m_debugMessenger, VK_NULL_HANDLE, m_DynamicDispatchInstance );
#endif
		m_instance.destroy();
		AZHAL_LOG_INFO( "vulkan instance destroyed" );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Renderer::CreateInstance()
	{
		vk::ApplicationInfo app_info
		{
			.pNext = VK_NULL_HANDLE,
			.pApplicationName = "Azhal Renderer Sandbox",
			.applicationVersion = VK_MAKE_VERSION( 0, 0, 1 ),
			.pEngineName = "Azhal",
			.apiVersion = VK_API_VERSION_1_3
		};

		std::vector<const char*> validation_layers {};
		if( m_validationLayersEnabled )
		{
			validation_layers = GetValidationLayers();
		}

		const std::vector<const char*>& required_extensions = GetRequiredExtensions();

		vk::InstanceCreateInfo instance_create_info
		{
			.pNext = VK_NULL_HANDLE,
			.pApplicationInfo = &app_info,
			.enabledLayerCount = VK_SIZE_CAST( validation_layers.size() ),
			.ppEnabledLayerNames = validation_layers.data(),
			.enabledExtensionCount = VK_SIZE_CAST( required_extensions.size() ),
			.ppEnabledExtensionNames = required_extensions.data()
		};

#ifdef AZHAL_ENABLE_LOGGING
		const vk::DebugUtilsMessengerCreateInfoEXT& debug_utils_create_info =
			GetDebugUtilsMessengerCreateInfo( reinterpret_cast< PFN_vkDebugUtilsMessengerCallbackEXT >( DebugCallback ) );

		const vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT> instance_create_chain
		{
			instance_create_info,
			debug_utils_create_info
		};
#else 
		const vk::StructureChain<vk::InstanceCreateInfo> instance_create_chain { instance_create_info };
#endif

		auto instance_create_rv = vk::createInstance( instance_create_chain.get<vk::InstanceCreateInfo>() );
		m_instance = CheckResultValue( instance_create_rv, "failed to create instance" );

		m_DynamicDispatchInstance = vk::DispatchLoaderDynamic( m_instance, vkGetInstanceProcAddr );

#ifdef AZHAL_ENABLE_LOGGING
		auto debug_msgnr_rv = m_instance.createDebugUtilsMessengerEXT( debug_utils_create_info, VK_NULL_HANDLE, m_DynamicDispatchInstance );
		m_debugMessenger = CheckResultValue( debug_msgnr_rv, "failed to create debug messenger" );
#endif
	}

	std::vector<const char*> Renderer::GetRequiredExtensions() const
	{
		Uint32 glfw_extension_count = 0;
		const char** ppGlfw_extensions = glfwGetRequiredInstanceExtensions( &glfw_extension_count );

		std::vector<const char*> glfw_extensions( ppGlfw_extensions, ppGlfw_extensions + glfw_extension_count );

#ifdef AZHAL_ENABLE_LOGGING
		glfw_extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
#endif

		return glfw_extensions;
	}

	std::vector<const char*> Renderer::GetValidationLayers() const
	{
		std::vector<const char*> validation_layers;

		if( m_validationLayersEnabled )
			validation_layers.push_back( "VK_LAYER_CHRONOS_validation" );

		return validation_layers;
	}

	vk::DebugUtilsMessengerCreateInfoEXT Renderer::GetDebugUtilsMessengerCreateInfo( const PFN_vkDebugUtilsMessengerCallbackEXT& debug_callback_fn ) const
	{
		using  MessageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
		static constexpr std::array<MessageSeverity, 4>  msg_severities
		{
			MessageSeverity::eVerbose,
			MessageSeverity::eInfo,
			MessageSeverity::eWarning,
			MessageSeverity::eError
		};

		vk::DebugUtilsMessageSeverityFlagsEXT msg_severity_flags = static_cast< vk::DebugUtilsMessageSeverityFlagsEXT >( 0 );
		for( auto itr = msg_severities.crbegin(); itr != msg_severities.crend(); ++itr )
		{
			MessageSeverity severity = *itr;
			if( severity < m_debugMessageSeverity )
				break;

			msg_severity_flags |= severity;
		}

		vk::DebugUtilsMessengerCreateInfoEXT debug_msg_create_info
		{
			.messageSeverity = msg_severity_flags,
			.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
			.pfnUserCallback = debug_callback_fn
		};

		return debug_msg_create_info;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	VKAPI_ATTR vk::Bool32 VKAPI_CALL Renderer::DebugCallback( vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity, vk::DebugUtilsMessageTypeFlagBitsEXT message_type,
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData )
	{
		switch( message_severity )
		{
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
			AZHAL_LOG_TRACE( "{0}", pCallbackData->pMessage );
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
			AZHAL_LOG_INFO( "{0}", pCallbackData->pMessage );
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
			AZHAL_LOG_WARN( "{0}", pCallbackData->pMessage );
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
			AZHAL_LOG_ERROR( "{0}", pCallbackData->pMessage );
			break;
		default:
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