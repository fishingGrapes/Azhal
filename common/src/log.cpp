#include "log.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

std::shared_ptr<spdlog::logger> AzhalLogger::s_logger;

void AzhalLogger::Init( const char* loggerName )
{
	spdlog::set_pattern( "%^[%T] %n: %v%$" );

	s_logger = spdlog::stdout_color_mt( loggerName );
	s_logger->set_level( spdlog::level::trace );
}