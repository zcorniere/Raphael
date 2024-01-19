#pragma once

#include "Engine/Threading/ThreadPool.hxx"

extern class Engine* GEngine;

class Engine
{
public:
    Engine();
    ~Engine();

    ThreadPool& GetThreadPool()
    {
        return m_ThreadPool;
    }

public:
    bool ShouldExit() const;

    bool Initialisation();
    void Destroy();

    void PreTick();
    void PostTick();

public:
    ThreadPool m_ThreadPool;
};
