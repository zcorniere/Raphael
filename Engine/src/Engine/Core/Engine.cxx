#include "Engine/Core/Engine.hxx"

#include "Engine/Core/Log.hxx"
#include "Engine/Core/Window.hxx"

#include "Engine/Math/Math.hxx"

uint64 GFrameCounter = 0;

FEngine* GEngine = nullptr;

DECLARE_LOGGER_CATEGORY(Core, LogEngine, Info)

FEngine::FEngine()
{
    GEngine = this;

    // Would love to make it constexpr, but it's not possible with the current C++ standard
    // accessing a union member is not allowed in a constant expression context
    LOG(LogEngine, Trace, "Making sure the math still make sense: {} == {}", Math::RightVector,
        Math::Cross(Math::UpVector, Math::FrontVector));
    check(Math::RightVector == Math::Cross(Math::UpVector, Math::FrontVector));
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

Ref<RWorld> FEngine::CreateWorld()
{
    return Ref<RWorld>::Create();
}

void FEngine::SetWorld(Ref<RWorld> World)
{
    if (LoadedWorld == World)
    {
        return;
    }

    LoadedWorld = World;
}

Ref<RWorld> FEngine::GetWorld() const
{
    return LoadedWorld;
}
