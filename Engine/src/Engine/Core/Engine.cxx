#include "Engine/Core/Engine.hxx"

#include "Engine/Core/Log.hxx"

uint64 GFrameCounter = 0;

Engine* GEngine = nullptr;

Engine::Engine()
{
}

Engine::~Engine()
{
}

bool Engine::ShouldExit() const
{
    return false;
}

bool Engine::Initialisation()
{
    m_ThreadPool.Start();

    return true;
}

void Engine::Destroy()
{
    m_ThreadPool.Stop();
}

void Engine::PreTick()
{
}

void Engine::PostTick()
{
}
