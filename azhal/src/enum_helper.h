#pragma once
#include <spdlog/fmt/bundled/format.h>

namespace gdevice
{
	constexpr const AnsiChar* EnumToString( vk::DebugUtilsMessageTypeFlagBitsEXT enum_val )
	{
		switch( enum_val )
		{
		case vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral:
			return "vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral";
		case vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation:
			return "vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation";
		case vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance:
			return "vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance";
		case vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding:
			return "vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding";
		default:
			return "Invalid vk::DebugUtilsMessageTypeFlagBitsEXT Enum";
		}
	}

	constexpr const AnsiChar* EnumToString( vk::DebugUtilsMessageSeverityFlagBitsEXT enum_val )
	{
		switch( enum_val )
		{
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
			return "vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose";
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
			return "vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo";
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
			return "vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning";
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
			return "vk::DebugUtilsMessageSeverityFlagBitsEXT::eError";
		default:
			return "Invalid vk::DebugUtilsMessageSeverityFlagBitsEXT Enum";
		}
	}

	constexpr const AnsiChar* EnumToString( vk::QueueFlagBits enum_val )
	{
		switch( enum_val )
		{
		case vk::QueueFlagBits::eGraphics:
			return "vk::QueueFlagBits::eGraphics";
		case vk::QueueFlagBits::eCompute:
			return "vk::QueueFlagBits::eCompute";
		case vk::QueueFlagBits::eTransfer:
			return "vk::QueueFlagBits::eTransfer";
		case vk::QueueFlagBits::eSparseBinding:
			return "vk::QueueFlagBits::eSparseBinding";
		case vk::QueueFlagBits::eProtected:
			return "vk::QueueFlagBits::eProtected";
		case vk::QueueFlagBits::eOpticalFlowNV:
			return "vk::QueueFlagBits::eOpticalFlowNV";
		default:
			return "Invalid vk::QueueFlagBits Enum";
		}
	}

}

// Note: As a rule of thumb, use EnumToString directly for values known at compile-time 
#define IMPLEMENT_FORMATTER_FOR_ENUM(ENUM_TYPE)                                        \
template<>                                                                             \
struct fmt::formatter<ENUM_TYPE> : formatter<std::string_view>                         \
{                                                                                      \
	template <typename FormatContext>                                                  \
	auto format( ENUM_TYPE enum_val, FormatContext& ctx ) -> decltype( ctx.out() ) {   \
		return formatter<string_view>::format( gdevice::EnumToString( enum_val ), ctx ); \
	}                                                                                  \
}                                                                                      \

IMPLEMENT_FORMATTER_FOR_ENUM( vk::DebugUtilsMessageTypeFlagBitsEXT );
IMPLEMENT_FORMATTER_FOR_ENUM( vk::DebugUtilsMessageSeverityFlagBitsEXT );
IMPLEMENT_FORMATTER_FOR_ENUM( vk::QueueFlagBits );