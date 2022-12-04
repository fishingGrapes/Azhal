#pragma once

#include "spdlog/logger.h"
#include "spdlog/fmt/ostr.h"

class CAzhalLog
{
public:
	CAzhalLog() = delete;
	static void Init(const char* loggerName);

	inline static std::shared_ptr<spdlog::logger> GetLogger()
	{
		return s_logger;
	};

private:
	static std::shared_ptr<spdlog::logger> s_logger;
};


#if defined(AZHAL_DEBUG) || defined(AZHAL_RELEASE)

#define AZHAL_LOG_TRACE(...)	  CAzhalLog::GetLogger()->trace(__VA_ARGS__)
#define AZHAL_LOG_INFO(...)       CAzhalLog::GetLogger()->info(__VA_ARGS__)
#define AZHAL_LOG_WARN(...)       CAzhalLog::GetLogger()->warn(__VA_ARGS__)
#define AZHAL_LOG_ERROR(...)      CAzhalLog::GetLogger()->error(__VA_ARGS__)
#define AZHAL_LOG_CRITICAL(...)   CAzhalLog::GetLogger()->critical(__VA_ARGS__)

#else

#define AZHAL_LOG_TRACE(...)	    
#define AZHAL_LOG_INFO(...)       
#define AZHAL_LOG_WARN(...)       
#define AZHAL_LOG_ERROR(...)      
#define AZHAL_LOG_CRITICAL(...)   

#endif