#pragma once
#include <vulkan/vulkan.hpp>

namespace azhal
{
	template <typename T>
	AZHAL_INLINE T get_vk_result( const vk::ResultValue<T>& res_val, const AnsiChar* fail_msg = "" )
	{
		if( res_val.result != vk::Result::eSuccess )
		{
			AZHAL_LOG_ALWAYS_ENABLED( "[CheckVkResultValue] {0}", fail_msg );
			AZHAL_FATAL_ASSERT( false, fail_msg );
		}

		return res_val.value;
	}

	template <typename T>
	AZHAL_INLINE T get_vk_result( const vk::ResultValue<T>& res_val, std::initializer_list<vk::Result> successCodes, const AnsiChar* fail_msg = "" )
	{
		if( std::find( successCodes.begin(), successCodes.end(), res_val.result ) != successCodes.end() )
		{
			AZHAL_LOG_ALWAYS_ENABLED( "[CheckVkResultValue] {0}", fail_msg );
			AZHAL_FATAL_ASSERT( false, fail_msg );
		}

		return res_val.value;
	}


}