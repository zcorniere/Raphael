#include "Engine/Core/Engine.hxx"

#include "Engine/Core/Log.hxx"
#include "Engine/Core/RHI/GenericRHI.hxx"
#include "Engine/Core/RHI/RHI.hxx"

Engine *GEngine = nullptr;

Engine::Engine(const int ac, const char *const *const av)
{
    (void)ac;
    (void)av;
    Log::Init();
}

Engine::~Engine()
{
    // Make sure no RObjects are left undestroyed
    // Not strictly necessary, but this precaution don't hurt ¯\_(ツ)_/¯
    check(RObjectUtils::AreThereAnyLiveObject() == false);

    Log::Shutdown();
}

bool Engine::Initialisation(IApplication *Application)
{
    checkNoReentry();

    App = Application;
    check(App);

    RHI::CreateRHI();
    GDynamicRHI->Init();
    m_ThreadPool.Start();

    return App->OnEngineInitialization();
}

void Engine::Destroy()
{
    App->OnEngineDestruction();
    m_ThreadPool.Stop();
    RHI::DeleteRHI();
}

unsigned Engine::Run()
{
    checkNoReentry();

    float dt = 0.0f;
    while (!App->ShouldExit()) {
        RHI::NextFrame();
        RHI::BeginFrame();

        auto startTime = std::chrono::high_resolution_clock::now();

        App->Tick(dt);

        auto stopTime = std::chrono::high_resolution_clock::now();
        dt = std::chrono::duration<float>(stopTime - startTime).count();

        RHI::EndFrame();
    }
    return 0;
}