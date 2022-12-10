#pragma once

#include "Engine/Core/Events/Event.hxx"
#include "Engine/Core/Events/WindowEvent.hxx"
#include "Engine/Core/TimeStep.hxx"
#include "Engine/Core/Window/Window.hxx"
#include "Engine/Renderer/Renderer.hxx"

#include <queue>

namespace Raphael
{
struct ApplicationSpecification {
    std::string Name = "Raphael";
    std::uint32_t WindowWidth = 1600;
    std::uint32_t WindowHeight = 900;
    bool WindowDecorated = true;
    bool Fullscreen = false;
    bool VSync = true;
    std::string WorkingDirectory;
    bool StartMaximized = true;
    bool Resizable = true;

    RendererConfig rendererConfig;
};

class Application
{
private:
    using EventCallbackFn = std::function<void(Event &)>;

public:
    static inline Application &Get()
    {
        return *s_Instance;
    }

public:
    Application(const ApplicationSpecification &specification);
    virtual ~Application();

    void Run();
    void Close();

    virtual void OnInit()
    {
    }
    virtual void OnShutdown();
    virtual void OnUpdate(Timestep ts)
    {
        (void)ts;
    }

    virtual void OnEvent(Event &event);

    template <typename Func>
    void QueueEvent(Func &&func)
    {
        m_EventQueue.push(func);
    }

    /// Creates & Dispatches an event either immediately, or adds it to an event queue which will be proccessed at the
    /// end of each frame
    template <typename TEvent, bool DispatchImmediately = false, typename... TEventArgs>
    void DispatchEvent(TEventArgs &&...args)
    {
        static_assert(std::is_assignable_v<Event, TEvent>);

        std::shared_ptr<TEvent> event = std::make_shared<TEvent>(std::forward<TEventArgs>(args)...);
        if constexpr (DispatchImmediately) {
            OnEvent(*event);
        } else {
            std::scoped_lock<std::mutex> lock(m_EventQueueMutex);
            m_EventQueue.push([event]() { Application::Get().OnEvent(*event); });
        }
    }

    Window &GetWindow()
    {
        return *m_Window;
    }

private:
    void ProcessEvents();

    bool OnWindowResize(WindowResizeEvent &e);
    bool OnWindowMinimize(WindowMinimizeEvent &e);
    bool OnWindowClose(WindowCloseEvent &e);

private:
    std::unique_ptr<Window> m_Window;
    ApplicationSpecification m_Specification;
    bool m_Running = true;
    bool m_Minimized = false;

    std::mutex m_EventQueueMutex;
    std::queue<std::function<void()>> m_EventQueue;
    std::vector<EventCallbackFn> m_EventCallbacks;

private:
    static Application *s_Instance;
};

}    // namespace Raphael
