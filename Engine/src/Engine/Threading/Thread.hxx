#pragma once

#include <string>
#include <thread>

#include "Engine/Threading/ThreadRuntime.hxx"

///
/// @brief Wrapper of the std::jthread
///
///
class FThread
{
public:
    ///
    /// @brief Construct a new Thread object
    ///
    FThread();

    FThread(FThread& other) = delete;
    /// Default move ctor
    FThread(FThread&& other) = default;

    /// Default move assignment
    FThread& operator=(FThread&& other) = default;
    FThread& operator=(FThread& other) = delete;

    virtual ~FThread();
    /// Initialize the thread
    void Start();
    ///
    /// @brief the internal thread should exit
    ///
    /// @param bShouldWait Does the thread should exit now or shutdown gracefully ?
    void End(bool bShouldWait = true);

    ///
    /// @brief Create the actual thread object
    ///
    /// @param name the name of the new thread
    /// @param threadCode The runtime to be executed in the thread
    void Create(const std::string& name, std::unique_ptr<IThreadRuntime> threadCode);

protected:
    /// Executed from the thread when started
    virtual void PreRun();
    /// Run the Interface::ThreadRuntime
    virtual std::uint32_t Run();
    /// Executed from the thread when quitting
    virtual void PostRun();

private:
    static void thread_runtime(FThread* pThis);

private:
    std::string m_name;
    std::jthread m_managedThread;
    std::unique_ptr<IThreadRuntime> m_internalRuntime;
};
