#include "Engine/Core/Log.hxx"

#include <cpplogger/formatters/ColorFormatter.hpp>
#include <cpplogger/sinks/StdoutSink.hpp>
#include <cstdio>

namespace Raphael
{

std::unique_ptr<cpplogger::Logger> s_CoreLogger = nullptr;

void Log::Init()
{
    s_CoreLogger = std::make_unique<cpplogger::Logger>("Core");

    s_CoreLogger->addSink(std::make_unique<cpplogger::StdoutSink>(stdout),
                          std::make_unique<cpplogger::ColorFormatter>());
}

void Log::Shutdown()
{
    s_CoreLogger.reset();
}

}    // namespace Raphael
