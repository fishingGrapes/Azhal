#pragma once

#define  VK_LAYER_KHRONOS_VALIDATION_NAME "VK_LAYER_KHRONOS_validation"

namespace azhal
{

	template <typename T>
	AZHAL_INLINE T CheckVkResultValue( const vk::ResultValue<T>& res_val, const AnsiChar* fail_msg )
	{
		if( res_val.result != vk::Result::eSuccess )
		{
			AZHAL_LOG_CRITICAL( "[CheckVkResultValue] {0}", fail_msg );
			AZHAL_DEBUG_BREAK();
		}

		return res_val.value;
	}

	template <typename T>
	AZHAL_INLINE T CheckVkResultValue( const vk::ResultValue<T>& res_val, const AnsiChar* fail_msg, std::initializer_list<vk::Result> successCodes )
	{
		if( std::find( successCodes.begin(), successCodes.end(), res_val.result ) != successCodes.end() )
		{
			AZHAL_LOG_CRITICAL( "[CheckVkResultValue] {0}", fail_msg );
			AZHAL_DEBUG_BREAK();
		}

		return res_val.value;
	}

}