#include "Engine/Core/Application.hxx"

namespace Raphael
{

Application *Application::s_Instance = nullptr;

Application::Application(const ApplicationSpecification &specification): m_Specification(specification)
{
    logger.start();
    Platform::setThreadName(logger.getThreadHandle(), "Logger Thread");
    s_Instance = this;

    if (!m_Specification.WorkingDirectory.empty()) { std::filesystem::current_path(m_Specification.WorkingDirectory); }

    Renderer::SetConfig(m_Specification.rendererConfig);

    WindowSpecification windowSpec;
    windowSpec.Title = m_Specification.Name;
    windowSpec.Width = m_Specification.WindowWidth;
    windowSpec.Height = m_Specification.WindowHeight;
    windowSpec.Decorated = m_Specification.WindowDecorated;
    windowSpec.Fullscreen = m_Specification.Fullscreen;
    windowSpec.VSync = m_Specification.VSync;
    m_Window = std::unique_ptr<Window>(Window::Create(windowSpec));
    m_Window->Init();
    m_Window->SetEventCallback([this](Event &e) { OnEvent(e); });
    if (m_Specification.StartMaximized)
        m_Window->Maximize();
    else
        m_Window->CenterWindow();
    m_Window->SetResizable(m_Specification.Resizable);

    Renderer::Init();
}

Application::~Application()
{
    m_Window->SetEventCallback([](Event &) {});

    Renderer::Shutdown();
    s_Instance = nullptr;
}

void Application::Run()
{
    OnInit();
    while (m_Running) {
        static std::uint64_t frameCounter = 0;
        ProcessEvents();

        if (!m_Minimized) { frameCounter++; }
    }
    OnShutdown();
}

void Application::Close()
{
    m_Running = false;
}

void Application::OnShutdown()
{
    m_EventCallbacks.clear();
}

void Application::ProcessEvents()
{
    m_Window->ProcessEvents();

    std::scoped_lock<std::mutex> lock(m_EventQueueMutex);

    while (m_EventQueue.size() > 0) {
        auto &func = m_EventQueue.front();
        func();
        m_EventQueue.pop();
    }
}

void Application::OnEvent(Event &event)
{
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent &e) { return OnWindowResize(e); });
    dispatcher.Dispatch<WindowMinimizeEvent>([this](WindowMinimizeEvent &e) { return OnWindowMinimize(e); });
    dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent &e) { return OnWindowClose(e); });

    for (auto &eventCallback: m_EventCallbacks) {
        eventCallback(event);
        if (event.Handled) break;
    }
}

bool Application::OnWindowResize(WindowResizeEvent &e)
{
    const std::uint32_t width = e.GetWidth();
    const std::uint32_t height = e.GetHeight();

    if (width == 0 || height == 0) { return false; }
    // TODO: On resize swapchain
    return false;
}

bool Application::OnWindowMinimize(WindowMinimizeEvent &e)
{
    m_Minimized = e.IsMinimized();
    return false;
}

bool Application::OnWindowClose(WindowCloseEvent &)
{
    Close();
    return false;    // give other things a chance to react to window close
}

}    // namespace Raphael
