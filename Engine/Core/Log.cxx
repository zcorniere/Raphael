#include "Engine/Core/Log.hxx"

#include <cpplogger/formatters/ColorFormatter.hpp>
#include <cpplogger/sinks/StdoutSink.hpp>
#include <cstdio>

cpplogger::Logger *s_CoreLogger = nullptr;

void Log::Init()
{
    if (!s_CoreLogger) {
        s_CoreLogger = new cpplogger::Logger("Core");

        s_CoreLogger->addSink(std::make_unique<cpplogger::StdoutSink>(stdout),
                              std::make_unique<cpplogger::ColorFormatter>());
    }
}

void Log::Shutdown()
{
    delete s_CoreLogger;
    s_CoreLogger = nullptr;
}
