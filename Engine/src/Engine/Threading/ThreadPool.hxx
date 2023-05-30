#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <queue>
#include <thread>
#include <type_traits>

#include "Engine/Threading/Thread.hxx"
#include "Engine/Threading/ThreadRuntime.hxx"

/// Manage a set of thread for sheduling work
class ThreadPool
{
    RPH_NONCOPYABLE(ThreadPool)
private:
    using WorkUnits = std::function<void(unsigned id)>;

    struct State {
        std::mutex q_mutex;
        std::condition_variable q_var;
        std::queue<WorkUnits> qWork;
    };

    class WorkerPoolRuntime : public ThreadRuntime
    {
    public:
        WorkerPoolRuntime(std::shared_ptr<ThreadPool::State> context);

        bool Init() override;
        std::uint32_t Run() override;
        void Stop() override;
        void Exit() override;

    private:
        static std::atomic_int s_threadIDCounter;

    private:
        int i_threadID;
        std::atomic_bool b_requestExit;
        std::shared_ptr<ThreadPool::State> p_state;
    };

public:
    ThreadPool();
    ~ThreadPool() = default;

    /// Create the pool with a given number of thread (default 2 / 3 of the max number of thread)
    void Start(unsigned size = std::max(((std::thread::hardware_concurrency() * 2) / 3), 1u));
    /// Stop and join all of the thread
    void Stop();
    /// Return the amount of thread in the pool
    unsigned Size()
    {
        return thread_p.Size();
    }
    /// Resize the thread pool
    void Resize(unsigned size);

    template <class F, typename... Args>
    /// Push a new job in the pool and return a future
    requires std::is_invocable_v<F, unsigned, Args...>
    [[nodiscard]] auto Push(F&& f, Args&&... args) -> std::future<decltype(f(0, args...))>
    {
        if (!verifyAlwaysMsg(!thread_p.empty(), "Pushing task when no thread are started !")) {
            Start(1);
        }
        auto packagedFunction = std::make_shared<std::packaged_task<decltype(f(0, args...))(unsigned)>>(
            std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Args>(args)...));

        WorkUnits storageFunc([packagedFunction](int id) { (*packagedFunction)(id); });

        {
            std::unique_lock lock(state->q_mutex);
            state->qWork.push(storageFunc);
        }
        state->q_var.notify_one();
        return packagedFunction->get_future();
    }

private:
    std::shared_ptr<State> state;
    Array<Thread> thread_p;
};
