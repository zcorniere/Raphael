#include "Engine/Core/Engine.hxx"

#include "Engine/Core/Log.hxx"
#include "Engine/Core/RHI/GenericRHI.hxx"
#include "Engine/Core/RHI/RHI.hxx"

Engine *GEngine = nullptr;

Engine::Engine(IApplication *Application, const int ac, const char *const *const av): App(Application)
{
    (void)ac;
    (void)av;
    Log::Init();

    check(App);
}

Engine::~Engine()
{
    // Make sure no RObjects are left undestroyed
    // Not strictly necessary, but this precaution don't hurt ¯\_(ツ)_/¯
    check(RObjectUtils::AreThereAnyLiveObject() == false);

    Log::Shutdown();
}

bool Engine::Initialisation()
{
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
