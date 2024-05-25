#include "Engine/Core/Application.hxx"
#include "Engine/Core/Engine.hxx"
#include "Engine/Core/Log.hxx"
#include "Engine/Core/RHI/GenericRHI.hxx"
#include "Engine/Core/RHI/RHI.hxx"
#include "Engine/Misc/CommandLine.hxx"
#include "Engine/Misc/Utils.hxx"

extern "C" IApplication* GetApplication();

int EngineLoop()
{
    GEngine = new Engine;

    if (!GEngine->Initialisation()) {
        return -1;
    }

    // Initialize the graphics RHI
    RHI::Create();
    GDynamicRHI->Init();

    IApplication* const Application = GetApplication();
    check(Application);
    if (!Application->OnEngineInitialization()) {
        return -1;
    }

    int ExitStatus = 0;
    float DeltaTime = 0.0f;
    while (!Utils::HasRequestedExit(ExitStatus) || GEngine->ShouldExit()) {
        RPH_PROFILE_FUNC("Engine Tick")
        const auto startTime = std::chrono::high_resolution_clock::now();

        GEngine->PreTick();
        RHI::BeginFrame();

        Application->Tick(DeltaTime);

        // Tick the RHI
        RHI::Tick(DeltaTime);

        GEngine->PostTick();

        // End the frame on the RHI side
        RHI::EndFrame();

        const auto stopTime = std::chrono::high_resolution_clock::now();
        DeltaTime = std::chrono::duration<float>(stopTime - startTime).count();
        // Must be on the last line of the engine loop
        RPH_PROFILE_MARK_FRAME
    }
    // Only destroy if the return value is ok
    if (ExitStatus == 0) {
        RHI::RHIWaitUntilIdle();
        Application->OnEngineDestruction();
        RHI::Destroy();
        GEngine->Destroy();
    }

    delete Application;
    delete GEngine;

    return ExitStatus;
}

int main(int ac, char** av)
{
    CommandLine::Set(ac, av);

    if (CommandLine::Param("-waitfordebugger")) {
        while (!Platform::isDebuggerPresent()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        PLATFORM_BREAK();
    }
    Platform::Initialize();
    Log::Init();

    const int GuardedReturnValue = EngineLoop();

    // Make sure no RObjects are left undestroyed
    // Not strictly necessary, but this precaution don't hurt ¯\_(ツ)_/¯
    check(RObjectUtils::AreThereAnyLiveObject() == false);

    Log::Shutdown();
    Platform::Deinitialize();
    return GuardedReturnValue;
}
