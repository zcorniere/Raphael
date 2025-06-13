#pragma once

#include "Engine/Core/Events/ApplicationEvent.hxx"
#include "Engine/Core/RHI/Resources/RHIViewport.hxx"
#include "Engine/Core/Window.hxx"

class IApplication : public RTTI::FEnable
{
    RTTI_DECLARE_TYPEINFO(IApplication)
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
    virtual void Tick(const double DeltaTime) = 0;
};

class FBaseApplication : public IApplication
{
    RTTI_DECLARE_TYPEINFO(FBaseApplication, IApplication)
public:
    virtual ~FBaseApplication()
    {
    }

    virtual bool OnEngineInitialization() override;
    virtual void OnEngineDestruction() override;

    virtual void Tick(const double DeltaTime) override;

protected:
    virtual void WindowEventHandler(FEvent& Event);

private:
    virtual bool OnWindowResize(FWindowResizeEvent& e);
    virtual bool OnWindowMinimize(FWindowMinimizeEvent& e);
    virtual bool OnWindowClose(FWindowCloseEvent& e);

protected:
    bool bShouldExit = false;

    Ref<RWindow> MainWindow;
    Ref<RRHIViewport> MainViewport;
};
