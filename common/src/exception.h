#pragma once
#include <exception>

class  GDeviceException : public std::exception
{
public:
	GDeviceException() = delete;
	GDeviceException( const char* msg )
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
