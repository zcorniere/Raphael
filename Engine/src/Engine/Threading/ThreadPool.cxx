#include "Engine/Threading/ThreadPool.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogWorkerThreadRuntime, Warning);

ThreadPool::ThreadPool(): state(std::make_shared<ThreadPool::State>())
{
}

void ThreadPool::Start(unsigned i)
{
    Resize(i);
}

void ThreadPool::Stop()
{
    state->q_var.notify_all();
    thread_p.Clear();
}

void ThreadPool::Resize(unsigned size)
{
    unsigned old_size = thread_p.Size();
    thread_p.Resize(size);
    for (; old_size < thread_p.Size(); old_size++) {
        thread_p[old_size].Create(std::format("Worker Thread nb {}", old_size),
                                  std::make_unique<WorkerPoolRuntime>(state));
    }
}

std::atomic_int ThreadPool::WorkerPoolRuntime::s_threadIDCounter = 0;

ThreadPool::WorkerPoolRuntime::WorkerPoolRuntime(std::shared_ptr<ThreadPool::State> context)
    : i_threadID(0), b_requestExit(false), p_state(std::move(context))
{
}

bool ThreadPool::WorkerPoolRuntime::Init()
{
    i_threadID = s_threadIDCounter++;
    return true;
}

std::uint32_t ThreadPool::WorkerPoolRuntime::Run()
{
    using namespace std::chrono_literals;
    ThreadPool::WorkUnits work;

    while (!b_requestExit) {
        try {
            {
                std::unique_lock lock(p_state->q_mutex);
                if (p_state->qWork.empty())
                    p_state->q_var.wait_for(lock, 10ms);
                /// lock is owned by this thread when .wait return

                if (p_state->qWork.empty())
                    continue;

                work = std::move(p_state->qWork.front());
                p_state->qWork.pop();
            }
            if (work)
                work(i_threadID);
        } catch (const std::exception& e) {
            LOG(LogWorkerThreadRuntime, Fatal, "{} : {}", i_threadID, e.what());
        } catch (...) {
            LOG(LogWorkerThreadRuntime, Fatal, "Unknown error on thread {}", i_threadID);
            return 1;
        }
    }
    return 0;
}

void ThreadPool::WorkerPoolRuntime::Stop()
{
    b_requestExit = true;
    LOG(LogWorkerThreadRuntime, Info, "Thread {}: exit requested", i_threadID);
}

void ThreadPool::WorkerPoolRuntime::Exit()
{
    b_requestExit = true;
    LOG(LogWorkerThreadRuntime, Info, "Thread {}: exit requested", i_threadID);
}
