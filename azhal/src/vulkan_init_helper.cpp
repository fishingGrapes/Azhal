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
		std::vector<vk::ValidationFeatureEnableEXT> validation_features;

		if( enable_gpu_assisted_validation )
		{
			validation_features.push_back( vk::ValidationFeatureEnableEXT::eGpuAssisted );
			validation_features.push_back( vk::ValidationFeatureEnableEXT::eBestPractices );
			// Enabling both eGpuAssited and eDebugPrintf would result in a validation error
			//validation_features.push_back(vk::ValidationFeatureEnableEXT::eDebugPrintf);
			validation_features.push_back( vk::ValidationFeatureEnableEXT::eSynchronizationValidation );
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

		vk::DebugUtilsMessengerCreateInfoEXT debug_msg_create_info
		{
			.messageSeverity = msg_severity_flags,
			.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
			.pfnUserCallback = debug_callback_fn
		};

		return debug_msg_create_info;
	}


	std::vector<const AnsiChar*> get_required_device_extensions()
	{
		static const std::vector<const AnsiChar*> required_device_extensions
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
			VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME
		};

		return required_device_extensions;
	}


	vk::SwapchainCreateInfoKHR build_swapchain_create_info( const vk::PhysicalDevice& physical_device, const vk::SurfaceKHR& surface, vk::Extent2D desired_extent )
	{
		const vk::ResultValue rv_surface_caps = physical_device.getSurfaceCapabilitiesKHR( surface );
		const vk::SurfaceCapabilitiesKHR& surface_caps = azhal::get_vk_result( rv_surface_caps, "failed to get surface capabilities" );

		const vk::ResultValue rv_surface_formats = physical_device.getSurfaceFormatsKHR( surface );
		const std::vector<vk::SurfaceFormatKHR>& surface_formats = azhal::get_vk_result( rv_surface_formats, "failed to get surface formats" );

		const vk::ResultValue rv_surface_present_modes = physical_device.getSurfacePresentModesKHR( surface );
		const std::vector<vk::PresentModeKHR>& surface_present_modes = azhal::get_vk_result( rv_surface_present_modes, "failed to get present modes" );

		AZHAL_FATAL_ASSERT( !surface_formats.empty() && !surface_present_modes.empty(), "swapchain support is not adequate" );

		// choose format and color-space
		auto [format, color_space] = [&surface_formats]() -> std::tuple<vk::Format, vk::ColorSpaceKHR>
		{
			for( const vk::SurfaceFormatKHR& format : surface_formats )
			{
				if( format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear )
				{
					return std::tuple( vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear );
				}
			}

			AZHAL_LOG_ALWAYS_ENABLED( "Failed to get appropriate format and colorspace for swapchain. fallback the first one." );
			return std::tuple( surface_formats[ 0 ].format, surface_formats[ 0 ].colorSpace );
		}( );

		// choose present mode
		vk::PresentModeKHR present_mode = [&surface_present_modes]() -> vk::PresentModeKHR
		{
			for( const vk::PresentModeKHR& present_mode : surface_present_modes )
			{
				if( present_mode == vk::PresentModeKHR::eMailbox )
				{
					return vk::PresentModeKHR::eMailbox;
				}

			}

			AZHAL_LOG_ALWAYS_ENABLED( "failed to get appropriate present mode for swapchain. fallback to FIFO mode." );
			return vk::PresentModeKHR::eFifo;
		}( );

		// choose swapchain extent
		vk::Extent2D swapchain_extent = [&desired_extent, &surface_caps]() -> vk::Extent2D
		{
			// if the current extent width is see to the special value UINT32_MAX, then the extent of the swapchain
			// can differ from the window resolution
			if( surface_caps.currentExtent.width != UINT32_MAX )
			{
				return surface_caps.currentExtent;
			}

			return {
				std::clamp<Uint32>( desired_extent.width, surface_caps.minImageExtent.width, surface_caps.maxImageExtent.width ),
				std::clamp<Uint32>( desired_extent.height, surface_caps.minImageExtent.height, surface_caps.maxImageExtent.height )
			};
		}( );


		Uint32 image_count = ( surface_caps.minImageCount + 1 );
		// if the maxImageCount has a special value of NULL/0, it means there is no max image count limit
		if( surface_caps.maxImageCount != NULL )
			image_count = std::min<Uint32>( image_count, surface_caps.maxImageCount );

		vk::SwapchainCreateInfoKHR swapchain_create_info
		{
			.surface = surface,
			.minImageCount = image_count,
			.imageFormat = format,
			.imageColorSpace = color_space,
			.imageExtent = swapchain_extent,
			.imageArrayLayers = 1,
			.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
			.imageSharingMode = vk::SharingMode::eExclusive,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = VK_NULL_HANDLE,
			.preTransform = surface_caps.currentTransform,
			.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
			.presentMode = present_mode,
			.clipped = VK_TRUE,
			.oldSwapchain = VK_NULL_HANDLE
		};

		return swapchain_create_info;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace azhal
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

		const std::vector<const AnsiChar*>& validation_layers = get_validation_layers( instance_creation_params.enableValidationLayers );
		const std::vector<const AnsiChar*>& required_extensions = get_required_instance_extensions();

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
		const std::vector<vk::ValidationFeatureEnableEXT>& enabled_validation_features = get_enabled_validation_features( instance_creation_params.enableGpuAssistedValidation );
		const vk::ValidationFeaturesEXT validation_features_info
		{
			.enabledValidationFeatureCount = VK_SIZE_CAST( enabled_validation_features.size() ),
			.pEnabledValidationFeatures = enabled_validation_features.data(),
			.disabledValidationFeatureCount = 0,
			.pDisabledValidationFeatures = VK_NULL_HANDLE
		};

		const vk::DebugUtilsMessengerCreateInfoEXT& debug_utils_create_info =
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
		const vk::Instance& instance = get_vk_result( rv_instance_create, "failed to create instance" );
		AZHAL_LOG_WARN( "vulkan instance created" );

		return instance;
	}


	vk::DebugUtilsMessengerEXT create_debug_messenger( const vk::Instance& instance, vk::DebugUtilsMessageSeverityFlagBitsEXT debug_message_severity,
		const PFN_vkDebugUtilsMessengerCallbackEXT& debug_callback_fn, const vk::DispatchLoaderDynamic& dynamic_dispatch_loader )
	{
		const vk::DebugUtilsMessengerCreateInfoEXT& debug_utils_create_info = build_debug_messenger_create_info( debug_message_severity, debug_callback_fn );

		const vk::ResultValue rv_debug_msgnr = instance.createDebugUtilsMessengerEXT( debug_utils_create_info, VK_NULL_HANDLE, dynamic_dispatch_loader );
		const vk::DebugUtilsMessengerEXT& debug_messenger = get_vk_result( rv_debug_msgnr, "failed to create debug messenger" );

		return debug_messenger;
	}


	vk::SurfaceKHR create_vulkan_surface( const vk::Instance& instance, void* p_window )
	{
		VkSurfaceKHR surface = VK_NULL_HANDLE;

		VkResult result = glfwCreateWindowSurface( instance, static_cast< GLFWwindow* >( p_window ), VK_NULL_HANDLE, &surface );
		AZHAL_FATAL_ASSERT( result == VkResult::VK_SUCCESS, "failed to create window surface" );
		AZHAL_LOG_WARN( "vulkan surface created" );

		return vk::SurfaceKHR( surface );
	}


	vk::PhysicalDevice get_suitable_physical_device( const vk::Instance& instance )
	{
		const vk::ResultValue rv_physical_devices = instance.enumeratePhysicalDevices();
		const std::vector<vk::PhysicalDevice>& physical_devices = azhal::get_vk_result( rv_physical_devices, "failed to enumerate physical devices" );

		// TODO: check for appropriate physical device props
		const vk::PhysicalDevice selected_physical_device = physical_devices[ 0 ];

		const vk::ResultValue rv_extension_props = selected_physical_device.enumerateDeviceExtensionProperties();
		const std::vector<vk::ExtensionProperties>& extension_props = azhal::get_vk_result( rv_extension_props, "failed to get extension properties for device" );

		const std::vector<const AnsiChar*>& required_extensions = get_required_device_extensions();
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


	vk::Device create_device( const vk::Instance& instance, const vk::PhysicalDevice& physical_device, const vk::SurfaceKHR& surface, const vk::DispatchLoaderDynamic& dynamic_dispatch_loader )
	{
		const auto find_queue_family_fn = [&physical_device]( vk::QueueFlagBits queue_flag, Bool is_present_queue = false )  -> Uint32
		{
			const std::vector<vk::QueueFamilyProperties>& queue_family_props = physical_device.getQueueFamilyProperties();
			for( Uint32 i = 0; i < queue_family_props.size(); ++i )
			{
				if( queue_family_props[ i ].queueFlags & queue_flag )
				{
					return i;
				}
			}

			AZHAL_LOG_ALWAYS_ENABLED( "Failed to find an appropriate queue family for the given queue flag, {0}", queue_flag );
			throw AzhalException( "Failed to find queue family" );
		};

		const auto find_present_queue_family_fn = [&physical_device, &surface, &dynamic_dispatch_loader]()  -> Uint32
		{
			const std::vector<vk::QueueFamilyProperties>& queue_family_props = physical_device.getQueueFamilyProperties();
			for( Uint32 i = 0; i < queue_family_props.size(); ++i )
			{
				const vk::ResultValue rv_surface_support = physical_device.getSurfaceSupportKHR( i, surface, dynamic_dispatch_loader );
				if( rv_surface_support.result == vk::Result::eSuccess )
				{
					return rv_surface_support.value;
				}
			}

			AZHAL_LOG_ALWAYS_ENABLED( "Failed to find  present queue family for the given queue flag" );
			throw AzhalException( "Failed to find present queue family" );
		};

		const Uint32 graphics_queue_family = find_queue_family_fn( vk::QueueFlagBits::eGraphics );
		const Uint32 compute_queue_family = find_queue_family_fn( vk::QueueFlagBits::eCompute );
		const Uint32 transfer_queue_family = find_queue_family_fn( vk::QueueFlagBits::eTransfer );
		const Uint32 present_queue_family = find_present_queue_family_fn();

		const std::set<Uint32> unique_queue_families
		{
			graphics_queue_family,
			compute_queue_family,
			transfer_queue_family,
			present_queue_family
		};

		std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
		for( Uint32 queue_family : unique_queue_families )
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

		const std::vector<const AnsiChar*>& required_extensions = get_required_device_extensions();

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
		const vk::Device& device = get_vk_result( rv_device, "Failed to create vulkan device" );
		AZHAL_LOG_WARN( "vulkan device created" );

		return device;
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////


	Swapchain create_swapchain( const vk::PhysicalDevice& physical_device, const vk::Device& device, const vk::SurfaceKHR& surface, vk::Extent2D desired_extent )
	{
		const vk::SwapchainCreateInfoKHR& swapchain_create_info = build_swapchain_create_info( physical_device, surface, desired_extent );

		const vk::ResultValue rv_swapchain = device.createSwapchainKHR( swapchain_create_info );
		vk::SwapchainKHR vk_swapchain = get_vk_result( rv_swapchain, "failed to create swapchain" );
		AZHAL_LOG_WARN( "vulkan swapchain created" );

		const vk::ResultValue rv_swapchain_images = device.getSwapchainImagesKHR( vk_swapchain );
		const std::vector<vk::Image>& swapchain_images = get_vk_result( rv_swapchain_images, "failed to retrieve swapchain images" );

		std::vector<vk::ImageView> swapchain_imageviews;
		swapchain_imageviews.reserve( swapchain_images.size() );
		for( const vk::Image& image : swapchain_images )
		{
			vk::ImageViewCreateInfo image_view_create_info
			{
				.image = image,
				.viewType = vk::ImageViewType::e2D,
				.format = swapchain_create_info.imageFormat,

				.components =
				{
					.r = vk::ComponentSwizzle::eIdentity,
					.g = vk::ComponentSwizzle::eIdentity,
					.b = vk::ComponentSwizzle::eIdentity,
					.a = vk::ComponentSwizzle::eIdentity
				},

				.subresourceRange =
				{
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				}
			};

			const vk::ResultValue rv_image_view = device.createImageView( image_view_create_info );
			const vk::ImageView image_view = get_vk_result( rv_image_view, "failed to create image view from swapchain image" );

			swapchain_imageviews.push_back( image_view );
		}
		AZHAL_LOG_WARN( "vulkan created swapchain image views" );

		const Swapchain swapchain
		{
			.vkSwapchain = vk_swapchain,
			.images = swapchain_images,
			.imageViews = swapchain_imageviews,
			.imageExtent = swapchain_create_info.imageExtent,
			.imageFormat = swapchain_create_info.imageFormat,
			.imageColorSpace = swapchain_create_info.imageColorSpace,
			.presentMode = swapchain_create_info.presentMode,
		};

		return swapchain;
	}


	void destroy_swapchain( const vk::Device& device, Swapchain& swapchain )
	{
		for( const vk::ImageView& image_view : swapchain.imageViews )
		{
			device.destroy( image_view );
		}
		
		swapchain.imageViews.clear();
		swapchain.images.clear();

		device.destroy( swapchain.vkSwapchain );
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////
}