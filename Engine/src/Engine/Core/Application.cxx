#include "Engine/Core/Application.hxx"

#include "Application.hxx"
#include "Engine/Core/RHI/RHI.hxx"
#include "Engine/Core/RHI/Resources/RHIViewport.hxx"
#include "Engine/Core/Window.hxx"

#include <GLFW/glfw3.h>

#include "Engine/Platforms/PlatformMisc.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogBaseApplication, Info)

bool BaseApplication::OnEngineInitialization()
{
    RPH_PROFILE_FUNC()

    WindowDefinition WindowDef{
        .AppearsInTaskbar = true,
        .Title = "Raphael Engine",
        .EventCallback = [this](Event& event) { WindowEventHandler(event); },
    };
    MainWindow = Ref<Window>::Create();
    MainWindow->SetName("MainWindow");
    MainWindow->Initialize(WindowDef);
    MainWindow->Show();
    MainWindow->Maximize();

    MainViewport = RHI::CreateViewport(MainWindow, glm::uvec2{500u, 500u});
    MainViewport->SetName("MainViewport");
    return true;
}

void BaseApplication::OnEngineDestruction()
{
    RPH_PROFILE_FUNC()

    MainViewport = nullptr;
    MainWindow->Destroy();
}

void BaseApplication::WindowEventHandler(Event& Event)
{
    EventDispatcher dispatcher(Event);
    dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& Event) { return OnWindowResize(Event); });
    dispatcher.Dispatch<WindowMinimizeEvent>([this](WindowMinimizeEvent& Event) { return OnWindowMinimize(Event); });
    dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& Event) { return OnWindowClose(Event); });

    if (!Event.Handled) {
        LOG(LogBaseApplication, Trace, "Unhandled Event : {}", Event);
    }
}

void BaseApplication::Tick(const float DeltaTime)
{
    RPH_PROFILE_FUNC()

    (void)DeltaTime;

    // Process All event
    ProcessEvents();
}

bool BaseApplication::ShouldExit() const
{
    return bShouldExit;
}

bool BaseApplication::OnWindowResize(WindowResizeEvent& E)
{
    RPH_PROFILE_FUNC()

    const uint32 width = E.GetWidth();
    const uint32 height = E.GetHeight();
    if (width == 0 || height == 0) {
        return false;
    }
    // The viewport was not created yet
    if (MainViewport) {
        MainViewport->ResizeViewport(width, height);
    }
    return false;
}

bool BaseApplication::OnWindowMinimize(WindowMinimizeEvent& E)
{
    RPH_PROFILE_FUNC()

    (void)E;
    return false;
}
bool BaseApplication::OnWindowClose(WindowCloseEvent& E)
{
    RPH_PROFILE_FUNC()

    (void)E;
    bShouldExit = true;
    return false;
}

void BaseApplication::ProcessEvents()
{
    MainWindow->ProcessEvents();

    {
        std::scoped_lock lock(m_EventQueueMutex);
        // Process custom event queue
        for (std::function<void()>& Func: m_EventQueue) {
            Func();
        }
        m_EventQueue.Clear();
    }
}
