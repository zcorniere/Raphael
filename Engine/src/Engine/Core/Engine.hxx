#pragma once

#include "Engine/AssetRegistry/AssetRegistry.hxx"
#include "Engine/Core/ECS/World.hxx"
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

    void SetWorld(Ref<ecs::RWorld> World);
    Ref<ecs::RWorld> GetWorld() const
    {
        return LoadedWorld;
    }

public:
    FAssetRegistry AssetRegistry;
    FThreadPool m_ThreadPool;

private:
    Ref<ecs::RWorld> LoadedWorld = nullptr;
};
