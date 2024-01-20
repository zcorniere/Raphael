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
};

#include <Engine/Core/Events/ApplicationEvent.hxx>
#include <Engine/Core/RHI/Resources/RHIViewport.hxx>
#include <Engine/Core/Window.hxx>

class BaseApplication : public IApplication
{
public:
    virtual ~BaseApplication()
    {
    }

    virtual bool OnEngineInitialization() override;
    virtual void OnEngineDestruction() override;

    virtual void Tick(const float DeltaTime) override;

private:
    virtual void WindowEventHandler(Event& Event);

    virtual bool OnWindowResize(WindowResizeEvent& e);
    virtual bool OnWindowMinimize(WindowMinimizeEvent& e);
    virtual bool OnWindowClose(WindowCloseEvent& e);

protected:
    bool bShouldExit = false;

    Ref<Window> MainWindow;
    Ref<RHIViewport> MainViewport;
};
