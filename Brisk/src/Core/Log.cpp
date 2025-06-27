// INCLUDES
#include "Log.hpp"
//----------------
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/msvc_sink.h"
//------------------------------------------

namespace Brisk 
{
	std::shared_ptr<spdlog::logger> Log::m_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::m_AppLogger;

    std::shared_ptr<Brisk::ImGuiSink> Log::g_ImGuiSink;

	void Log::Init()
	{
        std::vector<spdlog::sink_ptr> coreSinks;
        std::vector<spdlog::sink_ptr> appSinks;

        // Terminal sink
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        consoleSink->set_pattern("%^[%T] %n: %v%$");

#if defined(_WIN32)
        auto msvcSink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
        msvcSink->set_pattern("[%T] %n: %v");
        coreSinks.push_back(msvcSink);
        appSinks.push_back(msvcSink);
#endif

        // Add ImGui sink
        g_ImGuiSink = std::make_shared<Brisk::ImGuiSink>();
        coreSinks.push_back(consoleSink);
        appSinks.push_back(consoleSink);
        coreSinks.push_back(g_ImGuiSink);
        appSinks.push_back(g_ImGuiSink);

        m_CoreLogger = std::make_shared<spdlog::logger>("BRISK", coreSinks.begin(), coreSinks.end());
        m_AppLogger = std::make_shared<spdlog::logger>("APP", appSinks.begin(), appSinks.end());

        spdlog::register_logger(m_CoreLogger);
        spdlog::register_logger(m_AppLogger);

        //m_CoreLogger = spdlog::stdout_color_mt("BRISK");
        m_CoreLogger->set_level(spdlog::level::trace);

        //m_AppLogger = spdlog::stdout_color_mt("APP");
        m_AppLogger->set_level(spdlog::level::trace);
	}
}
