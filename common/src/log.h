#pragma once

#include <spdlog/async_logger.h>
#include <spdlog/fmt/ostr.h>

#include "macros.h"

class AzhalLogger
{
public:
	AzhalLogger() = delete;
	static void Init( const char* loggerName, const char* output_path = "logs" );

	AZHAL_INLINE static std::shared_ptr<spdlog::async_logger> Get()
	{
		return s_logger;
	};

private:
	static std::shared_ptr<spdlog::async_logger> s_logger;
};


#ifdef AZHAL_ENABLE_LOGGING

#define AZHAL_LOG_TRACE(...)	  AzhalLogger::Get()->trace(__VA_ARGS__)
#define AZHAL_LOG_INFO(...)       AzhalLogger::Get()->info(__VA_ARGS__)
#define AZHAL_LOG_WARN(...)       AzhalLogger::Get()->warn(__VA_ARGS__)
#define AZHAL_LOG_ERROR(...)      AzhalLogger::Get()->error(__VA_ARGS__)
#define AZHAL_LOG_CRITICAL(...)   AzhalLogger::Get()->critical(__VA_ARGS__)

#else

#define AZHAL_LOG_TRACE(...)	    
#define AZHAL_LOG_INFO(...)       
#define AZHAL_LOG_WARN(...)       
#define AZHAL_LOG_ERROR(...)      
#define AZHAL_LOG_CRITICAL(...)   

#endif

#define AZHAL_LOG_ALWAYS_ENABLED(...)   AzhalLogger::Get()->critical(__VA_ARGS__)
