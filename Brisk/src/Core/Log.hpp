#pragma once

// INCLUDES
#pragma warning(push, 0)
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#pragma warning(pop)
//------------------

namespace Brisk 
{
	class Log {
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return m_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetAppLogger() { return m_AppLogger; }
	private:
		static std::shared_ptr<spdlog::logger> m_CoreLogger;
		static std::shared_ptr<spdlog::logger> m_AppLogger;
	};
}

#if _DEBUG
//Core Log Macors
#define BRISK_CORE_TRACE(...) ::Brisk::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define BRISK_CORE_INFO(...)  ::Brisk::Log::GetCoreLogger()->info(__VA_ARGS__)
#define BRISK_CORE_WARN(...)  ::Brisk::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define BRISK_CORE_ERROR(...) ::Brisk::Log::GetCoreLogger()->error(__VA_ARGS__)

//Application Log Macors
#define BRISK_APP_TRACE(...) ::Brisk::Log::GetAppLogger()->trace(__VA_ARGS__)
#define BRISK_APP_INFO(...)  ::Brisk::Log::GetAppLogger()->info(__VA_ARGS__)
#define BRISK_APP_WARN(...)  ::Brisk::Log::GetAppLogger()->warn(__VA_ARGS__)
#define BRISK_APP_ERROR(...) ::Brisk::Log::GetAppLogger()->error(__VA_ARGS__)
#define BRISK_APP_FATAL(...) ::Brisk::Log::GetAppLogger()->critical(__VA_ARGS__)
#else 
//Core Log Macors
#define BRISK_CORE_TRACE(...)
#define BRISK_CORE_INFO(...) 
#define BRISK_CORE_WARN(...) 
#define BRISK_CORE_ERROR(...)

//Application Log Macors
#define BRISK_APP_TRACE(...)
#define BRISK_APP_INFO(...) 
#define BRISK_APP_WARN(...) 
#define BRISK_APP_ERROR(...)
#endif
