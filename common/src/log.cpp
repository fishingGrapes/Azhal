#include "log.h"

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <ctime>
#include <sstream>

namespace
{
	size_t MAX_FILE_SIZE = 1'048'576 * 5; // 5 Megabytes
	size_t MAX_FILE_COUNT = 10;
}

std::shared_ptr<spdlog::async_logger> AzhalLogger::s_logger;

void AzhalLogger::Init( const char* loggerName, const char* output_path )
{
	spdlog::init_thread_pool( 8192, 1 );

	// local time and date 
	// https://stackoverflow.com/questions/35258285/how-to-use-localtime-s-with-a-pointer-in-c
	const time_t timer = std::time( nullptr );
	struct tm current_time;
	localtime_s( &current_time, &timer );

	std::stringstream time_string_stream;
	time_string_stream << "..\\..\\..\\" << output_path << '\\' << loggerName << std::put_time( &current_time, "_%Y%m%d_%H%M%S.log" );

	const auto rotating_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>( time_string_stream.str(), MAX_FILE_SIZE, MAX_FILE_COUNT );
	rotating_file_sink->set_pattern( "%^[%T] %v%$" );
	const auto stdout_color_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	stdout_color_sink->set_pattern( "%^[%T] %v%$" );	

	std::vector<spdlog::sink_ptr> sinks { stdout_color_sink, rotating_file_sink };
#ifdef AZHAL_PLATFORM_WINDOWS
	sinks.push_back( std::make_shared<spdlog::sinks::msvc_sink_mt>() );
#endif

	s_logger = std::make_shared<spdlog::async_logger>( loggerName, std::begin( sinks ), std::end( sinks ), spdlog::thread_pool(), spdlog::async_overflow_policy::block );
	s_logger->set_level( spdlog::level::trace );
}