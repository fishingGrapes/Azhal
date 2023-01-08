#include "azpch.h"
#include "vulkan_init_helper.h"

#include "enums.h"
#include "window.h"

#include <GLFW/glfw3.h>
#include <ranges>

////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace
{
	constexpr const AnsiChar* VK_LAYER_KHRONOS_VALIDATION_NAME = "VK_LAYER_KHRONOS_validation";
}

namespace
{
	std::vector<const AnsiChar*> get_required_instance_extensions()
	{
		Uint32 glfw_extension_count = 0;
		const AnsiChar** pp_glfw_extensions = glfwGetRequiredInstanceExtensions( &glfw_extension_count );

		std::vector<const AnsiChar*> required_instance_extensions( pp_glfw_extensions, pp_glfw_extensions + glfw_extension_count );

#ifdef AZHAL_ENABLE_LOGGING
		required_instance_extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
#endif

		return required_instance_extensions;
	}


	std::vector<const AnsiChar*> get_validation_layers( Bool enable_validation_layers )
	{
		std::vector<const AnsiChar*> validation_layers;
		if( enable_validation_layers )
		{
			validation_layers.push_back( VK_LAYER_KHRONOS_VALIDATION_NAME );
		}
		return validation_layers;
	}


	std::vector<vk::ValidationFeatureEnableEXT> get_enabled_validation_features( Bool enable_gpu_assisted_validation )
	{
		std::vector<vk::ValidationFeatureEnableEXT> validation_features
		{
			 vk::ValidationFeatureEnableEXT::eBestPractices,
			 // Enabling both eGpuAssited and eDebugPrintf would result in a validation error
			 //vk::ValidationFeatureEnableEXT::eDebugPrintf,
			 vk::ValidationFeatureEnableEXT::eSynchronizationValidation
		};

		if( enable_gpu_assisted_validation )
		{
			validation_features.push_back( vk::ValidationFeatureEnableEXT::eGpuAssisted );
		}

		return validation_features;
	}


	vk::DebugUtilsMessengerCreateInfoEXT build_debug_messenger_create_info( vk::DebugUtilsMessageSeverityFlagBitsEXT msg_severity, const PFN_vkDebugUtilsMessengerCallbackEXT& debug_callback_fn )
	{
		using MessageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
		static constexpr std::array<MessageSeverity, 4>  msg_severities
		{
			MessageSeverity::eVerbose,
			MessageSeverity::eInfo,
			MessageSeverity::eWarning,
			MessageSeverity::eError
		};

		vk::DebugUtilsMessageSeverityFlagsEXT msg_severity_flags = static_cast< vk::DebugUtilsMessageSeverityFlagsEXT >( 0 );
		for( MessageSeverity severity : msg_severities | std::views::reverse )
		{
			if( severity < msg_severity )
				break;

			msg_severity_flags |= severity;
		}

		const vk::DebugUtilsMessengerCreateInfoEXT debug_msg_create_info
		{
			.messageSeverity = msg_severity_flags,
			.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
			.pfnUserCallback = debug_callback_fn
		};

		return debug_msg_create_info;
	}


	AZHAL_INLINE std::vector<const AnsiChar*> get_required_device_extensions()
	{
		static const std::vector<const AnsiChar*> required_device_extensions
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			// already included in the core 1.3
			//VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
			//VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME
		};

		return required_device_extensions;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace gdevice
{
	vk::Instance create_instance( const VulkanInstanceCreationParams& instance_creation_params )
	{
		const vk::ApplicationInfo app_info
		{
			.pNext = VK_NULL_HANDLE,
			.pApplicationName = "sandbox",
			.applicationVersion = VK_MAKE_VERSION( 0, 0, 1 ),
			.pEngineName = "azhal",
			.apiVersion = VK_API_VERSION_1_3
		};

		const std::vector<const AnsiChar*> validation_layers = get_validation_layers( instance_creation_params.enableValidationLayers );
		const std::vector<const AnsiChar*> required_extensions = get_required_instance_extensions();

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
		const std::vector<vk::ValidationFeatureEnableEXT> enabled_validation_features = get_enabled_validation_features( instance_creation_params.enableGpuAssistedValidation );
		const vk::ValidationFeaturesEXT validation_features_info
		{
			.enabledValidationFeatureCount = VK_SIZE_CAST( enabled_validation_features.size() ),
			.pEnabledValidationFeatures = enabled_validation_features.data(),
			.disabledValidationFeatureCount = 0,
			.pDisabledValidationFeatures = VK_NULL_HANDLE
		};

		const vk::DebugUtilsMessengerCreateInfoEXT debug_utils_create_info =
			build_debug_messenger_create_info( instance_creation_params.debugMessageSeverity, instance_creation_params.debugCallbackFn );

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
		const vk::Instance instance = get_vk_result( rv_instance_create, "failed to create instance" );

		return instance;
	}


	vk::DebugUtilsMessengerEXT create_debug_messenger( const vk::Instance instance, vk::DebugUtilsMessageSeverityFlagBitsEXT debug_message_severity,
		const PFN_vkDebugUtilsMessengerCallbackEXT& debug_callback_fn, const vk::DispatchLoaderDynamic& dynamic_dispatch_loader )
	{
		const vk::DebugUtilsMessengerCreateInfoEXT debug_utils_create_info = build_debug_messenger_create_info( debug_message_severity, debug_callback_fn );

		const vk::ResultValue rv_debug_msgnr = instance.createDebugUtilsMessengerEXT( debug_utils_create_info, VK_NULL_HANDLE, dynamic_dispatch_loader );
		const vk::DebugUtilsMessengerEXT debug_messenger = get_vk_result( rv_debug_msgnr, "failed to create debug messenger" );

		return debug_messenger;
	}


	vk::SurfaceKHR create_vulkan_surface( const vk::Instance instance, void* p_window )
	{
		VkSurfaceKHR surface = VK_NULL_HANDLE;

		VkResult result = glfwCreateWindowSurface( instance, static_cast< GLFWwindow* >( p_window ), VK_NULL_HANDLE, &surface );
		AZHAL_FATAL_ASSERT( result == VkResult::VK_SUCCESS, "failed to create window surface" );

		return vk::SurfaceKHR( surface );
	}


	Uint32 find_queue_family_index( const vk::PhysicalDevice physical_device, vk::QueueFlagBits queue_flag )
	{
		const std::vector<vk::QueueFamilyProperties> queue_family_props = physical_device.getQueueFamilyProperties();
		for( Uint32 i = 0; i < queue_family_props.size(); ++i )
		{
			if( queue_family_props[ i ].queueFlags & queue_flag )
			{
				return i;
			}
		}

		AZHAL_LOG_ALWAYS_ENABLED( "Failed to find an appropriate queue family for the given queue flag, {0}", queue_flag );
		throw GDeviceException( "Failed to find queue family" );

		return UINT32_MAX;
	}


	Uint32 find_present_queue_family_index( const vk::PhysicalDevice physical_device, const vk::SurfaceKHR surface, const vk::DispatchLoaderDynamic& dynamic_dispatch_loader )
	{
		const std::vector<vk::QueueFamilyProperties> queue_family_props = physical_device.getQueueFamilyProperties();
		for( Uint32 i = 0; i < queue_family_props.size(); ++i )
		{
			const vk::ResultValue rv_surface_support = physical_device.getSurfaceSupportKHR( i, surface, dynamic_dispatch_loader );
			const Bool has_present_support = get_vk_result( rv_surface_support, "failed to get surface support for physical device" );

			if( has_present_support )
			{
				return i;
			}
		}

		AZHAL_LOG_ALWAYS_ENABLED( "Failed to find  present queue family for the given queue flag" );
		throw GDeviceException( "Failed to find present queue family" );

		return UINT32_MAX;
	}


	vk::PhysicalDevice get_suitable_physical_device( const vk::Instance instance )
	{
		const vk::ResultValue rv_physical_devices = instance.enumeratePhysicalDevices();
		const std::vector<vk::PhysicalDevice> physical_devices = gdevice::get_vk_result( rv_physical_devices, "failed to enumerate physical devices" );

		// TODO: check for appropriate physical device props
		const vk::PhysicalDevice selected_physical_device = physical_devices[ 0 ];

		const vk::ResultValue rv_extension_props = selected_physical_device.enumerateDeviceExtensionProperties();
		const std::vector<vk::ExtensionProperties> extension_props = gdevice::get_vk_result( rv_extension_props, "failed to get extension properties for device" );

		const std::vector<const AnsiChar*> required_extensions = get_required_device_extensions();
		for( const AnsiChar* extension_name : required_extensions )
		{
			const auto iter = std::find_if
			(
				extension_props.begin(), extension_props.end(),

				[extension_name]( const vk::ExtensionProperties& prop ) -> Bool {
					return ( strcmp( prop.extensionName, extension_name ) == 0 );
				}
			);

			AZHAL_FATAL_ASSERT( iter != extension_props.cend(), "extension not found in physical device properties" );
		}

		return selected_physical_device;
	}


	vk::Device create_device( const vk::Instance instance, const vk::PhysicalDevice physical_device, const std::set<Uint32>& unique_queue_families )
	{
		std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
		for( Uint32 queue_family : unique_queue_families )
		{
			// TODO: queue_priority
			static const Float K_QUEUE_PRIORITY = 1.0f;
			const vk::DeviceQueueCreateInfo queue_create_info
			{
				.queueFamilyIndex = queue_family,
				.queueCount = 1,
				.pQueuePriorities = &K_QUEUE_PRIORITY
			};
			queue_create_infos.emplace_back( queue_create_info );
		}

		const std::vector<const AnsiChar*> required_extensions = get_required_device_extensions();

		const vk::DeviceCreateInfo device_create_info
		{
			.queueCreateInfoCount = VK_SIZE_CAST( queue_create_infos.size() ),
			.pQueueCreateInfos = queue_create_infos.data(),
			.enabledExtensionCount = VK_SIZE_CAST( required_extensions.size() ),
			.ppEnabledExtensionNames = required_extensions.data(),
			// TODO: add enabled features
			.pEnabledFeatures = VK_NULL_HANDLE
		};

		constexpr vk::PhysicalDeviceDynamicRenderingFeatures dynamic_render_features
		{
			.dynamicRendering = VK_TRUE
		};

		const vk::StructureChain<vk::DeviceCreateInfo, vk::PhysicalDeviceDynamicRenderingFeatures> device_create_chain
		{
			device_create_info,
			dynamic_render_features
		};

		const vk::ResultValue rv_device = physical_device.createDevice( device_create_chain.get<vk::DeviceCreateInfo>() );
		const vk::Device device = get_vk_result( rv_device, "Failed to create vulkan device" );

		return device;
	}
}