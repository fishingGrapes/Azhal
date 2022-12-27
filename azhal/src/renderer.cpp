#include "azpch.h"
#include "window.h"
#include "renderer.h"
#include "enums.h"

#include <GLFW/glfw3.h>

namespace azhal
{
	Renderer::Renderer( const RendererCreateInfo& renderer_create_info )
		: m_validationLayersEnabled( renderer_create_info.IsValidationLayersEnabled )
		, m_gpuAssistedValidationEnabled( renderer_create_info.IsGpuAssistedValidationEnabled )
		, m_debugMessageSeverity( renderer_create_info.DebugMessageSeverity )
	{
		CreateInstance();
		CreateSurface( renderer_create_info.pWindow );
		CreateDevice();
		CreateSwapchain( renderer_create_info.pWindow );
		Test();
	}

	void Renderer::Update()
	{
	}

	Renderer::~Renderer()
	{
		Destroy();
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

		const std::vector<const AnsiChar*>& validation_layers = GetValidationLayers();
		const std::vector<const AnsiChar*>& required_extensions = GetRequiredInstanceExtensions();

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
		const std::vector<vk::ValidationFeatureEnableEXT>& enabled_validation_features = GetEnabledValidationFeatures();
		const vk::ValidationFeaturesEXT validation_features_info
		{
			.enabledValidationFeatureCount = VK_SIZE_CAST( enabled_validation_features.size() ),
			.pEnabledValidationFeatures = enabled_validation_features.data(),
			.disabledValidationFeatureCount = 0,
			.pDisabledValidationFeatures = VK_NULL_HANDLE
		};

		const vk::DebugUtilsMessengerCreateInfoEXT& debug_utils_create_info =
			GetDebugUtilsMessengerCreateInfo( reinterpret_cast< PFN_vkDebugUtilsMessengerCallbackEXT >( DebugCallback ) );

		const vk::StructureChain<vk::InstanceCreateInfo, vk::ValidationFeaturesEXT, vk::DebugUtilsMessengerCreateInfoEXT> instance_create_chain
		{
			instance_create_info,
			validation_features_info,
			debug_utils_create_info
		};
#else 
		const vk::StructureChain<vk::InstanceCreateInfo> instance_create_chain { instance_create_info };
#endif

		const vk::ResultValue rv_instance_create = vk::createInstance( instance_create_chain.get<vk::InstanceCreateInfo>() );
		m_instance = CheckVkResultValue( rv_instance_create, "failed to create instance" );
		AZHAL_LOG_WARN( "vulkan instance created" );

		m_DynamicDispatchInstance = vk::DispatchLoaderDynamic( m_instance, vkGetInstanceProcAddr );

#ifdef AZHAL_ENABLE_LOGGING
		const vk::ResultValue rv_debug_msgnr = m_instance.createDebugUtilsMessengerEXT( debug_utils_create_info, VK_NULL_HANDLE, m_DynamicDispatchInstance );
		m_debugMessenger = CheckVkResultValue( rv_debug_msgnr, "failed to create debug messenger" );
#endif
	}

	void Renderer::CreateSurface( const WindowPtr& pWindow )
	{
		VkSurfaceKHR surface = VK_NULL_HANDLE;

		VkResult result = glfwCreateWindowSurface( m_instance, static_cast< GLFWwindow* >( pWindow->Get() ), VK_NULL_HANDLE, &surface );
		AZHAL_FATAL_ASSERT( result == VkResult::VK_SUCCESS, "failed to create window surface" );
		AZHAL_LOG_WARN( "vulkan surface created" );

		m_surface = surface;
	}

	void Renderer::CreateDevice()
	{
		m_device = RenderDevice( m_instance, m_surface, m_DynamicDispatchInstance );
	}

	void Renderer::CreateSwapchain( const WindowPtr& pWindow )
	{
		m_swapchain = m_device.CreateSwapchain( pWindow, m_surface );
	}

	// TODO: testing remove this
	void Renderer::Test()
	{
		const PSOCreateInfo pso_create_info
		{
			.vertexShader = AZHAL_FILE_PATH( "azhal/shaders/simple.vspv" ),
			.fragmentShader = AZHAL_FILE_PATH( "azhal/shaders/simple.pspv" )
		};

		const PSO& pso = m_device.CreatePSO( pso_create_info );
	}

	void Renderer::Destroy()
	{
		m_device.destroy( m_swapchain );
		m_device.destroy();

		m_instance.destroy( m_surface );
		AZHAL_LOG_WARN( "vulkan surface destroyed" );

#ifdef AZHAL_ENABLE_LOGGING
		m_instance.destroyDebugUtilsMessengerEXT( m_debugMessenger, VK_NULL_HANDLE, m_DynamicDispatchInstance );
#endif
		m_instance.destroy();
		AZHAL_LOG_WARN( "vulkan instance destroyed" );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::vector<const AnsiChar*> Renderer::GetRequiredInstanceExtensions() const
	{
		Uint32 glfw_extension_count = 0;
		const AnsiChar** ppGlfwExtensions = glfwGetRequiredInstanceExtensions( &glfw_extension_count );

		std::vector<const AnsiChar*> required_instance_extensions( ppGlfwExtensions, ppGlfwExtensions + glfw_extension_count );

#ifdef AZHAL_ENABLE_LOGGING
		required_instance_extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
#endif

		return required_instance_extensions;
	}

	std::vector<const AnsiChar*> Renderer::GetValidationLayers() const
	{
		std::vector<const AnsiChar*> validation_layers;

		if( m_validationLayersEnabled )
			validation_layers.push_back( VK_LAYER_KHRONOS_VALIDATION_NAME );

		return validation_layers;
	}

	std::vector<vk::ValidationFeatureEnableEXT> Renderer::GetEnabledValidationFeatures() const
	{
		if( !m_gpuAssistedValidationEnabled )
			return std::vector<vk::ValidationFeatureEnableEXT>();


		using enum vk::ValidationFeatureEnableEXT;
		std::vector<vk::ValidationFeatureEnableEXT> validation_features
		{
			eGpuAssisted,
			eBestPractices,
			// Enabling both eGpuAssited and eDebugPrintf would result in a validation error
			//eDebugPrintf,
			eSynchronizationValidation
		};

		return validation_features;
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