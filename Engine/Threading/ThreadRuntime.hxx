#pragma once

#include <cstdint>

///
/// @brief The internal runtime of a thread (ie: the code that will run on the thread)
///
class ThreadRuntime
{
public:
    ///
    /// @brief Destroy the IThreadRuntime object
    ///
    virtual ~ThreadRuntime()
    {
    }

    /// Initialize the runtime
    virtual bool init() = 0;
    /// Start the main loop of the thread
    virtual std::uint32_t run() = 0;
    /// Tell the thread that it should gracefully stop
    virtual void stop() = 0;
    /// Tell the thread to exit as soon as possible
    virtual void exit() = 0;
};
