#pragma once

#include "Engine/Threading/ThreadPool.hxx"

class IApplication
{
public:
    virtual ~IApplication() {}

    virtual bool OnEngineInitialization() = 0;
    virtual void OnEngineDestruction() = 0;

    virtual void Tick(const float DeltaTime) = 0;
    virtual bool ShouldExit() const = 0;
};

extern class Engine *GEngine;

class Engine
{
public:
    template <std::derived_from<IApplication> T>
    static int Start(const int ac, const char *const *const av)
    {
        GEngine = new Engine(ac, av);

        IApplication *Application = new T();
        if (!GEngine->Initialisation(Application)) { return -1; }

        unsigned Result = GEngine->Run();

        // Only destroy if the return value is ok
        if (Result == 0) { GEngine->Destroy(); }

        delete GEngine;
        delete Application;

        return Result;
    }

public:
    Engine() = delete;
    Engine(const int ac, const char *const *const av);
    ~Engine();

    ThreadPool &GetThreadPool() { return m_ThreadPool; }

private:
    bool Initialisation(IApplication *Application);
    void Destroy();

    unsigned Run();

public:
    IApplication *App;
    ThreadPool m_ThreadPool;
};
