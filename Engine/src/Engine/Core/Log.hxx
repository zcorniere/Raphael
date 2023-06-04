#pragma once

#include <cpplogger/Logger.hpp>

/// Class to manage the Log startup and shutdown sequence
class Log
{
public:
    /// @brief Start the Logger
    ///
    /// Must be called as soon a possible
    static void Init();

    /// @brief Shutdown the loggers
    static void Shutdown();
};
