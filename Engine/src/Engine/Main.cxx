#include "Engine/Core/Application.hxx"
#include "Engine/Core/Engine.hxx"
#include "Engine/Core/Log.hxx"
#include "Engine/Core/RHI/GenericRHI.hxx"
#include "Engine/Core/RHI/RHI.hxx"
#include "Engine/Misc/CommandLine.hxx"
#include "Engine/Misc/Utils.hxx"

#ifdef PLATFORM_WINDOWS
    #include <windows.h>
#endif    // PLATFORM_WINDOWS

DECLARE_LOGGER_CATEGORY(Core, LogEngine, Info)

extern "C" IApplication* GetApplication();

int EngineLoop()
try {
    GEngine = new FEngine;

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

    GDynamicRHI->PostInit();

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
        RHI::FlushDeletionQueue();

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
} catch (const std::exception& e) {
    LOG(LogEngine, Error, "EngineLoop exception: {:s}", e.what());
    return -1;
} catch (...) {
    LOG(LogEngine, Error, "EngineLoop unknown exception");
    return -1;
}

#ifdef PLATFORM_WINDOWS
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nShowCmd;
    FCommandLine::Set(GetCommandLine());
#else
int main(int ac, char** av)
{
    FCommandLine::Set(ac, av);
#endif    // !PLATFORM_WINDOWS

    FPlatform::Initialize();
    if (FCommandLine::Param("-waitfordebugger")) {
        while (!FPlatform::isDebuggerPresent()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        PLATFORM_BREAK();
    }
    Log::Init();

    const int GuardedReturnValue = EngineLoop();

    // Make sure no RObjects are left undestroyed
    // Not strictly necessary, but this precaution don't hurt ¯\_(ツ)_/¯
    check(RObjectUtils::AreThereAnyLiveObject() == false);

    Log::Shutdown();
    FPlatform::Deinitialize();
    return GuardedReturnValue;
}
