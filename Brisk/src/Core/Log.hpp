#pragma once

// INCLUDES
#pragma warning(push, 0)
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#pragma warning(pop)

#include "spdlog/sinks/base_sink.h"
#include <mutex>
#include <vector>
#include <string>

//------------------

#if defined(_WIN32)
#define BRISK_DEBUG_BREAK() __debugbreak()
#else
#include <signal.h>
#define BRISK_DEBUG_BREAK() raise(SIGTRAP)
#endif

namespace Brisk 
{
    struct ConsoleLogEntry {
        spdlog::level::level_enum level;
        std::string message;
    };

    class ImGuiSink : public spdlog::sinks::base_sink<std::mutex> {
    public:
        const std::vector<ConsoleLogEntry>& GetEntries() const { return m_Entries; }
        void Clear() { m_Entries.clear(); }

    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override {
            spdlog::memory_buf_t formatted;
            formatter_->format(msg, formatted);

            m_Entries.push_back(ConsoleLogEntry{
                msg.level,
                fmt::to_string(formatted)
                });
        }

        void flush_() override {}

    private:
        std::vector<ConsoleLogEntry> m_Entries;
    };


	class Log {
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return m_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetAppLogger() { return m_AppLogger; }
    public:
        static std::shared_ptr<ImGuiSink> g_ImGuiSink;
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

// ------------------------------------------------------------------------------------------------
// ASSERTIONS
// ------------------------------------------------------------------------------------------------
#define BRISK_ASSERT(cond, ...)                                  \
        do {                                                         \
            if (!(cond)) {                                           \
                BRISK_CORE_ERROR("Assertion Failed: " __VA_ARGS__);  \
                BRISK_DEBUG_BREAK();                                 \
            }                                                        \
        } while(0)

#define BRISK_ASSERT_MSG(cond, msg)                              \
        do {                                                         \
            if (!(cond)) {                                           \
                BRISK_CORE_ERROR("Assertion Failed: {}", msg);       \
                BRISK_DEBUG_BREAK();                                 \
            }                                                        \
        } while(0)

#define BRISK_CORE_ASSERT(cond, ...)                             \
        do {                                                         \
            if (!(cond)) {                                           \
                BRISK_CORE_ERROR("Assertion Failed: " __VA_ARGS__);  \
                BRISK_DEBUG_BREAK();                                 \
            }                                                        \
        } while(0)

#define BRISK_APP_ASSERT(cond, ...)                              \
        do {                                                         \
            if (!(cond)) {                                           \
                BRISK_APP_ERROR("Assertion Failed: " __VA_ARGS__);   \
                BRISK_DEBUG_BREAK();                                 \
            }                                                        \
        } while(0)

#else 
//Core Log Macors
#define BRISK_CORE_TRACE(...)
#define BRISK_CORE_INFO(...) 
#define BRISK_CORE_WARN(...) 
#define BRISK_CORE_ERROR(...)
#define BRISK_CORE_ASSERT(...)

//Application Log Macors
#define BRISK_APP_TRACE(...)
#define BRISK_APP_INFO(...) 
#define BRISK_APP_WARN(...) 
#define BRISK_APP_ERROR(...)
#endif