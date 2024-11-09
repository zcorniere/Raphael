#include "Engine/Core/Engine.hxx"

#include "Engine/Core/Log.hxx"
#include "Engine/Core/Window.hxx"

uint64 GFrameCounter = 0;

FEngine* GEngine = nullptr;

FEngine::FEngine()
{
    GEngine = this;
}

FEngine::~FEngine()
{
}

bool FEngine::ShouldExit() const
{
    return false;
}

bool FEngine::Initialisation()
{
    RWindow::EnsureGLFWInit();

    m_ThreadPool.Start();

    return true;
}

void FEngine::Destroy()
{
    m_ThreadPool.Stop();
}

void FEngine::PreTick()
{
}

void FEngine::PostTick()
{
}
