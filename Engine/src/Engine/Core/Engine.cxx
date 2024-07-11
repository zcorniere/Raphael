#include "Engine/Core/Engine.hxx"

#include "Engine/Core/Log.hxx"
#include "Engine/Core/Window.hxx"

uint64 GFrameCounter = 0;

Engine* GEngine = nullptr;

Engine::Engine()
{
    GEngine = this;
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
    Window::EnsureGLFWInit();

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
