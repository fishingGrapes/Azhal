#include "azpch.h"
#include "window.h"
#include "renderer.h"
#include "vulkanHelper.h"
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
		CreateDevice();
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
		const std::vector<const AnsiChar*>& required_extensions = GetRequiredExtensions();

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

		vk::ResultValue instance_create_rv = vk::createInstance( instance_create_chain.get<vk::InstanceCreateInfo>() );
		m_instance = CheckVkResultValue( instance_create_rv, "failed to create instance" );

		m_DynamicDispatchInstance = vk::DispatchLoaderDynamic( m_instance, vkGetInstanceProcAddr );

#ifdef AZHAL_ENABLE_LOGGING
		vk::ResultValue debug_msgnr_rv = m_instance.createDebugUtilsMessengerEXT( debug_utils_create_info, VK_NULL_HANDLE, m_DynamicDispatchInstance );
		m_debugMessenger = CheckVkResultValue( debug_msgnr_rv, "failed to create debug messenger" );
#endif
	}

	void Renderer::CreateDevice()
	{
		const vk::PhysicalDevice physical_device = GetSuitablePhysicalDevice();

		const auto find_queue_family_fn = [&physical_device]( vk::QueueFlagBits queue_flag, Bool is_present_queue = false )  -> QueueFamily
		{
			const std::vector<vk::QueueFamilyProperties>& queue_family_props = physical_device.getQueueFamilyProperties();
			for( Uint32 i = 0; i < queue_family_props.size(); ++i )
			{
				if( is_present_queue )
				{
					// TODO: add code for present queue selection
					//physical_device.getSurfaceSupportKHR()
				}

				if( queue_family_props[ i ].queueFlags & queue_flag )
				{
					return i;
				}
			}

			AZHAL_LOG_ALWAYS_ENABLED( "Failed to find an appropriate queue family for the given queue flag, {0}", queue_flag );
			throw AzhalException( "Failed to create queue family" );
		};

		const QueueFamily graphics_queue_family = find_queue_family_fn( vk::QueueFlagBits::eGraphics );
		const QueueFamily compute_queue_family = find_queue_family_fn( vk::QueueFlagBits::eCompute );
		const QueueFamily transfer_queue_family = find_queue_family_fn( vk::QueueFlagBits::eTransfer );

		const std::set<QueueFamily> unique_queue_families
		{
			graphics_queue_family,
			compute_queue_family,
			transfer_queue_family
		};

		std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
		for( QueueFamily queue_family : unique_queue_families )
		{
			// TODO: queue_priority
			static const Float QUEUE_PRIORITY = 1.0f;
			const vk::DeviceQueueCreateInfo queue_create_info
			{
				.queueFamilyIndex = queue_family,
				.queueCount = 1,
				.pQueuePriorities = &QUEUE_PRIORITY
			};
			queue_create_infos.emplace_back( queue_create_info );
		}

		vk::DeviceCreateInfo device_create_info
		{
			.queueCreateInfoCount = VK_SIZE_CAST( queue_create_infos.size() ),
			.pQueueCreateInfos = queue_create_infos.data(),
			// TODO: add enabled features
			.pEnabledFeatures = VK_NULL_HANDLE
		};

		vk::ResultValue device_rv = physical_device.createDevice( device_create_info );
		m_device = CheckVkResultValue( device_rv, "Failed to create vulkan device" );

		m_graphicsQueue = GpuQueue( m_device.getQueue( graphics_queue_family, 0 ), graphics_queue_family );
		m_computeQueue = GpuQueue( m_device.getQueue( compute_queue_family, 0 ), compute_queue_family );
		m_transferQueue = GpuQueue( m_device.getQueue( transfer_queue_family, 0 ), transfer_queue_family );
	}

	void Renderer::Destroy()
	{
		m_device.destroy();
		AZHAL_LOG_INFO( "vulkan device destroyed" );

#ifdef AZHAL_ENABLE_LOGGING
		m_instance.destroyDebugUtilsMessengerEXT( m_debugMessenger, VK_NULL_HANDLE, m_DynamicDispatchInstance );
#endif
		m_instance.destroy();
		AZHAL_LOG_INFO( "vulkan instance destroyed" );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::vector<const AnsiChar*> Renderer::GetRequiredExtensions() const
	{
		Uint32 glfw_extension_count = 0;
		const AnsiChar** ppGlfwExtensions = glfwGetRequiredInstanceExtensions( &glfw_extension_count );

		std::vector<const AnsiChar*> glfw_extensions( ppGlfwExtensions, ppGlfwExtensions + glfw_extension_count );

#ifdef AZHAL_ENABLE_LOGGING
		glfw_extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
#endif

		return glfw_extensions;
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

	vk::PhysicalDevice Renderer::GetSuitablePhysicalDevice() const
	{
		vk::ResultValue physical_devices_rv = m_instance.enumeratePhysicalDevices();
		const std::vector<vk::PhysicalDevice>& physical_devices = CheckVkResultValue( physical_devices_rv, "failed to enumerate physical devices" );

		// TODO: check for appropriate physical device props
		const vk::PhysicalDevice selected_physical_device = physical_devices[ 0 ];
		const vk::PhysicalDeviceProperties& physical_device_props = selected_physical_device.getProperties();

		return selected_physical_device;
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
		std::string message_type_name = "";
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