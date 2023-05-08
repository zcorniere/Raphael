#pragma once

#include <Engine/Core/Engine.hxx>
#include <Engine/Core/RHI/RHIResource.hxx>
#include <Engine/Core/Window.hxx>

#include <SDL3/SDL.h>

class EditorApplication : public IApplication
{
public:
    EditorApplication();
    ~EditorApplication();

    bool OnEngineInitialization() override;
    void OnEngineDestruction() override;

    void ProcessEvent(SDL_Event Event);

    void Tick(const float DeltaTime) override;
    bool ShouldExit() const override;

    Ref<Window> FindEventWindow(SDL_Event &Event);

private:
    bool bShouldExit = false;

    std::vector<Ref<Window>> Windows;
    Ref<RHIViewport> Viewport;
};
