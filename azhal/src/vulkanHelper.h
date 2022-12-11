#pragma once

namespace azhal
{
	template <typename T>
	AZHAL_INLINE T CheckVkResultValue( const vk::ResultValue<T>& res_val, const char* fail_msg )
	{
		if( res_val.result != vk::Result::eSuccess )
		{
			AZHAL_LOG_CRITICAL( "[CheckVkResultValue] {0}", fail_msg );
			AZHAL_DEBUG_BREAK();
		}

		return res_val.value;
	}

	template <typename T>
	AZHAL_INLINE T CheckVkResultValue( const vk::ResultValue<T>& res_val, const char* fail_msg, std::initializer_list<vk::Result> successCodes )
	{
		if( std::find( successCodes.begin(), successCodes.end(), res_val.result ) != successCodes.end() )
		{
			AZHAL_LOG_CRITICAL( "[CheckVkResultValue] {0}", fail_msg );
			AZHAL_DEBUG_BREAK();
		}

		return res_val.value;
	}
}