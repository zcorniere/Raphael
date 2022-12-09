#pragma once

#include <cpplogger/Logger.hpp>

#define RPH_TRACE_TAG(Tag, ...) ::Raphael::Log::PrintMessage(cpplogger::Level::Trace, Tag, __VA_ARGS__)
#define RPH_INFO_TAG(Tag, ...) ::Raphael::Log::PrintMessage(cpplogger::Level::Info, Tag, __VA_ARGS__)
#define RPH_WARN_TAG(Tag, ...) ::Raphael::Log::PrintMessage(cpplogger::Level::Warn, Tag, __VA_ARGS__)
#define RPH_ERROR_TAG(Tag, ...) ::Raphael::Log::PrintMessage(cpplogger::Level::Error, Tag, __VA_ARGS__)

namespace Raphael::Log
{

template <typename... Args>
void PrintMessage(cpplogger::Level level, const std::string &tag, Args... args)
{
    cpplogger::Stream stream = logger.level(level, tag);
    ((stream << args), ...);
}

}    // namespace Raphael::Log
