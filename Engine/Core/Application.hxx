#pragma once

#include "Engine/Core/Log.hxx"
#include "Engine/Core/RHI/GenericRHI.hxx"
#include "Engine/Core/RHI/RHIResource.hxx"
#include "Engine/Core/RObject.hxx"
#include "Engine/Core/Window.hxx"
#include "Engine/Platforms/Platform.hxx"
#include "Engine/Threading/ThreadPool.hxx"

#include <SDL3/SDL.h>

class Application
{
public:
    Application();
    ~Application();

    // Called on init, return false if there was an error
    bool Initialize();
    void Shutdown();

    void ProcessEvent(SDL_Event Event);

    void Tick(const float DeltaTime);
    bool ShouldExit() const;

    ThreadPool &GetThreadPool() { return m_ThreadPool; }

    Ref<Window> FindEventWindow(SDL_Event &Event);

private:
    ThreadPool m_ThreadPool;
    bool bShouldExit = false;
    Ref<RHI> m_RHI;

    std::vector<Ref<Window>> Windows;
    Ref<RHIViewport> Viewport;
};
