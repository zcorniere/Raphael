#include "Engine/Threading/ThreadPool.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogWorkerThreadRuntime, Info);

ThreadPool::ThreadPool(): state(std::make_shared<ThreadPool::State>())
{
}

void ThreadPool::start(unsigned i)
{
    resize(i);
}

void ThreadPool::stop()
{
    state->q_var.notify_all();
    thread_p.clear();
}

void ThreadPool::resize(unsigned size)
{
    unsigned old_size = thread_p.size();
    thread_p.resize(size);
    for (; old_size < thread_p.size(); old_size++) {
        thread_p.at(old_size).create("Worker Thread nb " + std::to_string(old_size),
                                     std::make_unique<WorkerPoolRuntime>(state));
    }
}

std::atomic_int ThreadPool::WorkerPoolRuntime::s_threadIDCounter = 0;

ThreadPool::WorkerPoolRuntime::WorkerPoolRuntime(std::shared_ptr<ThreadPool::State> context)
    : i_threadID(0), b_requestExit(false), p_state(std::move(context))
{
}

bool ThreadPool::WorkerPoolRuntime::init()
{
    i_threadID = s_threadIDCounter++;
    return true;
}

std::uint32_t ThreadPool::WorkerPoolRuntime::run()
{
    using namespace std::chrono_literals;
    ThreadPool::WorkUnits work;

    while (!b_requestExit) {
        try {
            {
                std::unique_lock lock(p_state->q_mutex);
                if (p_state->qWork.empty()) p_state->q_var.wait_for(lock, 10ms);
                /// lock is owned by this thread when .wait return

                if (p_state->qWork.empty()) continue;

                work = std::move(p_state->qWork.front());
                p_state->qWork.pop();
            }
            if (work) work(i_threadID);
        } catch (const std::exception &e) {
            LOG(LogWorkerThreadRuntime, Fatal, "{} : {}", i_threadID, e.what());
        } catch (...) {
            LOG(LogWorkerThreadRuntime, Fatal, "Unkown error on thread {}", i_threadID);
        }
    }
    return 0;
}

void ThreadPool::WorkerPoolRuntime::stop()
{
    b_requestExit = true;
    LOG(LogWorkerThreadRuntime, Info, "Thread {}: exit requested", i_threadID);
}

void ThreadPool::WorkerPoolRuntime::exit()
{
    b_requestExit = true;
    LOG(LogWorkerThreadRuntime, Info, "Thread {}: exit requested", i_threadID);
}
