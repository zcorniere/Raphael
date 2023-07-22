#include "Engine/Core/Log.hxx"

#include <cpplogger/formatters/ColorFormatter.hpp>
#include <cpplogger/sinks/StdoutSink.hpp>
#include <cstdio>

static cpplogger::Logger* s_CoreLogger = nullptr;

class RaphaelFormatter
{
public:
    static std::string format(const cpplogger::Message& message)
    {
        using namespace cpplogger;

        return std::format("[{0:%F} {0:%T}][{1}][{2}{3}{4}][{5}] {6}",
                           std::chrono::floor<std::chrono::milliseconds>(message.LogTime), GFrameCounter % 1000,
                           internal::color(levelColor(message.LogLevel)), to_string(message.LogLevel),
                           internal::resetSequence, message.CategoryName, message.Message);
    }
};

void Log::Init()
{
    if (!s_CoreLogger) {
        s_CoreLogger = new cpplogger::Logger("Core");

        s_CoreLogger->addSink<cpplogger::StdoutSink, RaphaelFormatter>(stdout);
    }
}

void Log::Shutdown()
{
    delete s_CoreLogger;
    s_CoreLogger = nullptr;
}
