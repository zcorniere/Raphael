#pragma once

#include "Engine/Threading/ThreadPool.hxx"

class IApplication
{
public:
    virtual ~IApplication()
    {
    }

    /// Called when the Engine created
    virtual bool OnEngineInitialization() = 0;
    /// Called when the Engine is destroyed
    virtual void OnEngineDestruction() = 0;

    /// Called once per frame
    /// @param DeltaTime the time elapsed since last frame
    virtual void Tick(const float DeltaTime) = 0;
    /// Determine if the application want the engine to exit
    virtual bool ShouldExit() const = 0;
};

extern class Engine* GEngine;

class Engine
{
public:
    /// @brief Start the Engine
    /// @tparam T The Application type to use
    /// @return The exit code of the engine.
    template <std::derived_from<IApplication> T>
    static int Start(const int ac, const char* const* const av)
    {
        GEngine = new Engine(ac, av);

        IApplication* Application = new T();
        if (!GEngine->Initialisation(Application)) {
            return -1;
        }

        unsigned Result = GEngine->Run();

        // Only destroy if the return value is ok
        if (Result == 0) {
            GEngine->Destroy();
        }

        delete GEngine;
        delete Application;

        return Result;
    }

public:
    Engine() = delete;
    Engine(const int ac, const char* const* const av);
    ~Engine();

    ThreadPool& GetThreadPool()
    {
        return m_ThreadPool;
    }

private:
    bool Initialisation(IApplication* Application);
    void Destroy();

    unsigned Run();

public:
    IApplication* App;
    ThreadPool m_ThreadPool;
};
