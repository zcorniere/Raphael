#pragma once

#include "Engine/Threading/ThreadPool.hxx"

extern class FEngine* GEngine;

class FEngine
{
public:
    FEngine();
    ~FEngine();

    FThreadPool& GetThreadPool()
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
    FThreadPool m_ThreadPool;
};
