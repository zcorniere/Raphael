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

    Math::RegisterMathTypes();
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
    RTTI::Registrar::Get().Init();

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
    if (LoadedWorld == World) {
        return;
    }

    LoadedWorld = World;
}

Ref<RWorld> FEngine::GetWorld() const
{
    return LoadedWorld;
}
