#include "Engine/Core/Application.hxx"

#include "Engine/Renderer/Vulkan/VulkanCommandsObjects.hxx"
#include "Engine/Renderer/Vulkan/VulkanDevice.hxx"
#include "Engine/Renderer/Vulkan/VulkanResources.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogApplication, Warn)

Application *GApplication = nullptr;

Application::Application()
{
    Log::Init();

    RHI = Ref<VulkanRHI::VulkanDynamicRHI>::Create();

    check(GApplication == nullptr);
    GApplication = this;
}

Application::~Application()
{
    RHI = nullptr;
    GApplication = nullptr;

    // Make sure no RObjects are left undestroyed
    // Not strictly necessary, but this precaution don't hurt ¯\_(ツ)_/¯
    check(RObjectUtils::AreThereAnyLiveObject() == false);

    Log::Shutdown();
}

bool Application::Initialize()
{
    RHI->Init();

    WindowDefinition WindowDef{
        .AppearsInTaskbar = true,
        .Title = "Raphael Engine",
    };
    Windows.push_back(Ref<Window>::Create());
    Windows[0]->SetName("Main Window");
    Windows[0]->Initialize(WindowDef, nullptr);
    Windows[0]->Show();

    Viewport =
        Ref<VulkanRHI::VulkanViewport>::Create(RHI.As<VulkanRHI::VulkanDynamicRHI>()->GetDevice(), Windows[0]->GetHandle(), glm::uvec2{500u, 500u});
    Viewport->SetName("Main viewport");
    return true;
}

void Application::Shutdown()
{
    Viewport = nullptr;

    for (Ref<Window> &Win: Windows) { Win->Destroy(); }
    Windows.clear();

    RHI->Shutdown();
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

    RHI::Submit([]() { check(true); });
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
