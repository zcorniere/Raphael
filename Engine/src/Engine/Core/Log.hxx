#pragma once

#include <cpplogger/Logger.hpp>

extern uint64 GFrameCounter;

/// Class to manage the Log startup and shutdown sequence
class Log
{
public:
    class ColorFormatter
    {
    public:
        static inline std::string format(const cpplogger::Message& message)
        {
            using namespace cpplogger;

            return std::format("[{0:%F} {0:%T}][{1:0>3}][{2:#}][{3}] {4}",
                               std::chrono::floor<std::chrono::milliseconds>(message.LogTime), GFrameCounter % 1000,
                               message.LogLevel, message.CategoryName, message.Message);
        }
    };

    class BaseFormatter
    {
    public:
        static inline std::string format(const cpplogger::Message& message)
        {
            using namespace cpplogger;

            return std::format("[{0:%F} {0:%T}][{1:0>3}][{2}][{3}] {4}",
                               std::chrono::floor<std::chrono::milliseconds>(message.LogTime), GFrameCounter % 1000,
                               message.LogLevel, message.CategoryName, message.Message);
        }
    };

public:
    /// @brief Start the Logger
    ///
    /// Must be called as soon a possible
    static void Init();

    /// @brief Shutdown the loggers
    static void Shutdown();
};
