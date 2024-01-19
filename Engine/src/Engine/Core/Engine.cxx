#include "Engine/Core/Engine.hxx"

#include "Engine/Core/Log.hxx"
#include "Engine/Core/RHI/GenericRHI.hxx"
#include "Engine/Core/RHI/RHI.hxx"

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
    checkNoReentry();

    RHI::Create();

    GDynamicRHI->Init();
    m_ThreadPool.Start();

    return true;
}

void Engine::Destroy()
{
    m_ThreadPool.Stop();
    RHI::Destroy();
}

void Engine::PreTick()
{
    RPH_PROFILE_FUNC();

    RHI::BeginFrame();
}

void Engine::PostTick()
{
    RHI::EndFrame();
    RHI::NextFrame();
}
