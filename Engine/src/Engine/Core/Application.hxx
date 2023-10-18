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
    virtual bool ShouldExit() const override;

    virtual void WindowEventHandler(Event& Event);

    /// Creates & Dispatches an event either immediately, or adds it to an event queue which will be proccessed at the
    /// end of each frame
    template <typename TEvent, bool DispatchImmediately = false, typename... TEventArgs>
    void DispatchEvent(TEventArgs&&... args)
    {
        static_assert(std::is_assignable_v<Event, TEvent>);

        std::shared_ptr<TEvent> event = std::make_shared<TEvent>(std::forward<TEventArgs>(args)...);
        if constexpr (DispatchImmediately) {
            WindowEventHandler(*event);
        } else {
            std::scoped_lock lock(m_EventQueueMutex);
            m_EventQueue.Add([this, event]() { WindowEventHandler(*event); });
        }
    }

private:
    void ProcessEvents();

    virtual bool OnWindowResize(WindowResizeEvent& e);
    virtual bool OnWindowMinimize(WindowMinimizeEvent& e);
    virtual bool OnWindowClose(WindowCloseEvent& e);

protected:
    bool bShouldExit = false;

    Ref<Window> MainWindow;
    Ref<RHIViewport> MainViewport;

    std::mutex m_EventQueueMutex;
    Array<std::function<void()>> m_EventQueue;
};
