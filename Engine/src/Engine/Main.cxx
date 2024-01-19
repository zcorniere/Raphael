#include "Engine/Core/Application.hxx"
#include "Engine/Core/Engine.hxx"
#include "Engine/Core/Log.hxx"
#include "Engine/Misc/Utils.hxx"
#include "Engine/Raphael.hxx"

extern "C" {
IApplication* GetApplication();
}

int EngineLoop()
{
    GEngine = new Engine;

    if (!GEngine->Initialisation()) {
        return -1;
    }

    IApplication* const Application = GetApplication();
    check(Application);
    if (!Application->OnEngineInitialization()) {
        return -1;
    }

    int ExitStatus = 0;
    while (!Utils::HasRequestedExit(ExitStatus) || GEngine->ShouldExit()) {

        GEngine->PreTick();

        Application->Tick(0.0f);

        GEngine->PostTick();

        RPH_PROFILE_MARK_FRAME
        GFrameCounter += 1;
    }
    // Only destroy if the return value is ok
    if (ExitStatus == 0) {
        Application->OnEngineDestruction();
        GEngine->Destroy();
    }

    delete Application;
    delete GEngine;

    return ExitStatus;
}

int main(int, char**)
{
    Platform::Initialize();
    Log::Init();

    const int GuardedReturnValue = EngineLoop();

    // Make sure no RObjects are left undestroyed
    // Not strictly necessary, but this precaution don't hurt ¯\_(ツ)_/¯
    check(RObjectUtils::AreThereAnyLiveObject() == false);

    Log::Shutdown();
    return GuardedReturnValue;
}
