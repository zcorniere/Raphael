#include "Engine/Core/Log.hxx"

#include "Engine/Platforms/PlatformMisc.hxx"

#include <cpplogger/sinks/FileSink.hpp>
#include <cpplogger/sinks/StdoutSink.hpp>
#include <cstdio>

static cpplogger::Logger* s_CoreLogger = nullptr;

void Log::Init()
{
    if (!s_CoreLogger) {
        s_CoreLogger = new cpplogger::Logger("Core");

        s_CoreLogger->addSink<cpplogger::StdoutSink, Log::ColorFormatter>(stdout);
    }
}

void Log::Shutdown()
{
    delete s_CoreLogger;
    s_CoreLogger = nullptr;
}
