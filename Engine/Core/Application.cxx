#include "Engine/Core/Application.hxx"

#include "Engine/Core/RHI/GenericRHI.hxx"

#include "Engine/Core/FrameGraph/FrameGraph.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogApplication, Warn)

Application *GApplication = nullptr;

Application::Application()
{
    Log::Init();

    m_RHI = Ref<RHI>::Create();

    check(GApplication == nullptr);
    GApplication = this;
}

Application::~Application()
{
    m_RHI = nullptr;
    GApplication = nullptr;

    // Make sure no RObjects are left undestroyed
    // Not strictly necessary, but this precaution don't hurt ¯\_(ツ)_/¯
    check(RObjectUtils::AreThereAnyLiveObject() == false);

    Log::Shutdown();
}

bool Application::Initialize()
{
    m_RHI->Init();

    WindowDefinition WindowDef{
        .AppearsInTaskbar = true,
        .Title = "Raphael Engine",
    };
    Windows.push_back(Ref<Window>::CreateNamed("Main Window"));
    Windows[0]->Initialize(WindowDef, nullptr);
    Windows[0]->Show();

    Viewport = RHI::Create<RHIResourceType::Viewport>((void *)Windows[0]->GetHandle(), glm::uvec2{500u, 500u});
    Viewport->SetName("Main viewport");

#if 1
    struct FrameData {
        FrameGraphResource Texture;
    };
    FrameGraph Graph;
    Graph.AddCallbackPass<FrameData>(
        "Test pass",
        [](FrameGraphBuilder &Builder, FrameData &Data) {
            RHITextureCreateDesc Description{};
            Data.Texture = Builder.Create<RHIResourceType::Texture>("Test Texture", Description);
            LOG(LogApplication, Info, "Setup");
        },
        [](const FrameData &, FrameGraphPassResources &) { LOG(LogApplication, Info, "Execution"); });
    Graph.Compile();
    Graph.Execute();
#endif

    return true;
}

void Application::Shutdown()
{
    Viewport = nullptr;

    for (Ref<Window> &Win: Windows) { Win->Destroy(); }
    Windows.clear();

    m_RHI->Shutdown();
}

void Application::ProcessEvent(SDL_Event SDLEvent)
{
    Ref<Window> EventWindow = FindEventWindow(SDLEvent);
    if (!EventWindow) { return; }

    switch (SDLEvent.type) {
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED: bShouldExit = true; break;
        default: break;
    }
}

void Application::Tick(const float DeltaTime)
{

    RHI::BeginFrame();

    (void)DeltaTime;
    SDL_Event event;
    // Process All event
    while (SDL_PollEvent(&event)) { ProcessEvent(event); }

    RHI::Submit([] { check(true); });
    RHI::EndFrame();

    RHI::NextFrame();
}

bool Application::ShouldExit() const { return bShouldExit; }

Ref<Window> Application::FindEventWindow(SDL_Event &Event)
{
    uint32 WindowID;

    switch (Event.type) {
        case SDL_EVENT_TEXT_INPUT: WindowID = Event.text.windowID; break;
        case SDL_EVENT_TEXT_EDITING: WindowID = Event.edit.windowID; break;
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: WindowID = Event.key.windowID; break;
        case SDL_EVENT_MOUSE_MOTION: WindowID = Event.motion.windowID; break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP: WindowID = Event.button.windowID; break;
        case SDL_EVENT_MOUSE_WHEEL: WindowID = Event.wheel.windowID; break;
        case SDL_EVENT_WINDOW_MOUSE_ENTER:
        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
        case SDL_EVENT_WINDOW_FOCUS_LOST:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        case SDL_EVENT_WINDOW_TAKE_FOCUS:
        case SDL_EVENT_WINDOW_HIT_TEST: WindowID = Event.window.windowID; break;
        default: return nullptr;
    }

    for (Ref<Window> &Window: Windows) {
        if (SDL_GetWindowID(Window->GetHandle()) == WindowID) { return Window; }
    }
    return nullptr;
}
