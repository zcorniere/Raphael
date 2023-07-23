#include "Engine/Core/Application.hxx"

#include "Application.hxx"
#include "Engine/Core/RHI/RHI.hxx"
#include "Engine/Core/RHI/Resources/RHIViewport.hxx"
#include "Engine/Core/Window.hxx"

#include <GLFW/glfw3.h>

DECLARE_LOGGER_CATEGORY(Core, LogBaseApplication, Info)

bool BaseApplication::OnEngineInitialization()
{
    RPH_PROFILE_FUNC()

    WindowDefinition WindowDef{
        .AppearsInTaskbar = true,
        .Title = "Raphael Engine",
        .EventCallback = [this](Event& event) { ProcessEvent(event); },
    };
    MainWindow = std::make_unique<Window>();
    MainWindow->SetName("Main Window");
    MainWindow->Initialize(WindowDef);
    MainWindow->Show();
    MainWindow->Maximize();

    MainViewport = RHI::CreateViewport((void*)MainWindow->GetHandle(), glm::uvec2{500u, 500u});
    MainViewport->SetName("Main viewport");

    // RHI::CreateGraphicsPipeline(RHIGraphicsPipelineInitializer{
    //     .VertexShader = "DefaultTriangle.vert",
    //     .PixelShader = "DefaultTriangle.frag",
    //     .Rasterizer =
    //         {
    //             .PolygonMode = EPolygonMode::Fill,
    //             .CullMode = ECullMode::Back,
    //             .FrontFaceCulling = EFrontFace::Clockwise,
    //         },
    // });
    return true;
}

void BaseApplication::OnEngineDestruction()
{
    RPH_PROFILE_FUNC()

    MainViewport = nullptr;
    MainWindow->Destroy();
}

void BaseApplication::ProcessEvent(Event& Event)
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
    const uint32 width = E.GetWidth();
    const uint32 height = E.GetHeight();
    if (width == 0 || height == 0) {
        return false;
    }
    MainViewport->ResizeViewport(width, height);
    return false;
}

bool BaseApplication::OnWindowMinimize(WindowMinimizeEvent& E)
{
    (void)E;
    return false;
}
bool BaseApplication::OnWindowClose(WindowCloseEvent& E)
{
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
