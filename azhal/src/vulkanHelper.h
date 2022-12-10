#pragma once

namespace azhal
{
	template <typename T>
	AZHAL_INLINE T CheckResultValue( const vk::ResultValue<T>& res_val, const char* fail_msg )
	{
		vk::resultCheck( res_val.result, fail_msg );
		return res_val.value;
	}
}