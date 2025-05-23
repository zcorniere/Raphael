#pragma once

#include <cstdint>

///
/// @brief The internal runtime of a thread (ie: the code that will run on the thread)
///
class IThreadRuntime
{
public:
    ///
    /// @brief Destroy the IThreadRuntime object
    ///
    virtual ~IThreadRuntime()
    {
    }

    /// Initialize the runtime
    virtual bool Init() = 0;
    /// Start the main loop of the thread
    virtual std::uint32_t Run() = 0;
    /// Tell the thread that it should gracefully stop
    virtual void Stop() = 0;
    /// Tell the thread to exit as soon as possible
    virtual void Exit() = 0;
};
