#include "Engine/Threading/Thread.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogThread, Warning)

Thread::Thread(): m_name(), m_internalRuntime(nullptr)
{
}

Thread::~Thread()
{
    End();
}

void Thread::Start()
{
}

void Thread::End(bool bShouldWait)
{
    if (m_internalRuntime) {
        m_internalRuntime->Stop();
    }
    m_managedThread.request_stop();

    if (bShouldWait && m_managedThread.joinable()) {
        m_managedThread.join();
    }
}

void Thread::Create(const std::string& name, std::unique_ptr<ThreadRuntime> threadCode)
{
    if (m_managedThread.joinable()) {
        m_managedThread.request_stop();
        m_managedThread.join();
    }

    m_internalRuntime = std::move(threadCode);
    m_name = name;

    m_managedThread = std::jthread(thread_runtime, this);
    Platform::setThreadName(m_managedThread, m_name);
}

void Thread::PreRun()
{
}

std::uint32_t Thread::Run()
{
    std::uint32_t exitCode = 1;
    check(m_internalRuntime);

    if (m_internalRuntime->Init()) {
        PreRun();
        exitCode = m_internalRuntime->Run();
    } else {
        m_internalRuntime->Stop();
    }

    LOG(LogThread, Info, "Thread (\"{}\") exited with code {}", m_name, exitCode);
    return exitCode;
}

void Thread::PostRun()
{
}

void Thread::thread_runtime(Thread* pThis)
{
    check(pThis);
    pThis->Start();
    pThis->Run();
    pThis->PostRun();
}
