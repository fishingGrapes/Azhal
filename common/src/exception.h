#pragma once
#include <exception>

namespace azhal
{
	class AzhalException : public std::exception
	{
	public:
		AzhalException() = delete;
		AzhalException( const char* msg )
			: m_msg( msg )
		{
		}

		AZHAL_INLINE const char* what()
		{
			return m_msg;
		}

	private:
		const char* m_msg;
	};
}