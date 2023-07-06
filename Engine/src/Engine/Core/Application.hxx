#pragma once

class IApplication
{
public:
    virtual ~IApplication()
    {
    }

    /// Called when the Engine created
    virtual bool OnEngineInitialization() = 0;
    /// Called when the Engine is destroyed
    virtual void OnEngineDestruction() = 0;

    /// Called once per frame
    /// @param DeltaTime the time elapsed since last frame
    virtual void Tick(const float DeltaTime) = 0;
    /// Determine if the application want the engine to exit
    virtual bool ShouldExit() const = 0;
};

#include <Engine/Core/Events.hxx>
#include <Engine/Core/RHI/RHIResource.hxx>
#include <Engine/Core/Window.hxx>

class BaseApplication : public IApplication
{
public:
    virtual ~BaseApplication()
    {
    }

    bool OnEngineInitialization() override;
    void OnEngineDestruction() override;

    void Tick(const float DeltaTime) override;
    bool ShouldExit() const override;

    virtual void ProcessEvent(const WindowEvent& Event);

protected:
    Ref<Window> CreateNewWindow(const std::string& Name = "");

private:
    WindowEvent ConvertWindowEvent(const SDL_Event& Event);

protected:
    bool bShouldExit = false;

    WeakRef<Window> MainWindow;
    Array<Ref<Window>> Windows;
    Ref<RHIViewport> MainViewport;
};