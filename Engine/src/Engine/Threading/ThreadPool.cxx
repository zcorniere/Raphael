#include "Engine/Threading/ThreadPool.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogWorkerThreadRuntime, Warning);

FThreadPool::FThreadPool(): state(std::make_shared<FThreadPool::State>())
{
}

void FThreadPool::Start(unsigned i)
{
    Resize(i);
}

void FThreadPool::Stop()
{
    state->q_var.notify_all();
    thread_p.Clear();
}

void FThreadPool::Resize(unsigned size)
{
    unsigned old_size = thread_p.Size();
    thread_p.Resize(size);
    for (; old_size < thread_p.Size(); old_size++) {
        std::unique_ptr<WorkerPoolRuntime> Runtime = std::make_unique<WorkerPoolRuntime>(state);
        thread_p[old_size].Create(std::format("Worker Thread nb {}", old_size), std::move(Runtime));
    }
}

std::atomic_int FThreadPool::WorkerPoolRuntime::s_threadIDCounter = 0;

FThreadPool::WorkerPoolRuntime::WorkerPoolRuntime(std::shared_ptr<FThreadPool::State> context)
    : i_threadID(0), b_requestExit(false), p_state(std::move(context))
{
}

bool FThreadPool::WorkerPoolRuntime::Init()
{
    i_threadID = s_threadIDCounter++;
    return true;
}

std::uint32_t FThreadPool::WorkerPoolRuntime::Run()
{
    using namespace std::chrono_literals;
    FThreadPool::WorkUnits work;

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

void FThreadPool::WorkerPoolRuntime::Stop()
{
    b_requestExit = true;
    LOG(LogWorkerThreadRuntime, Info, "Thread {}: exit requested", i_threadID);
}

void FThreadPool::WorkerPoolRuntime::Exit()
{
    b_requestExit = true;
    LOG(LogWorkerThreadRuntime, Info, "Thread {}: exit requested", i_threadID);
}
