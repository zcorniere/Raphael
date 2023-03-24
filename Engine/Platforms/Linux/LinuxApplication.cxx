#include "Engine/Platforms/Linux/LinuxApplication.hxx"

#include "Engine/Renderer/Vulkan/VulkanCommandsObjects.hxx"
#include "Engine/Renderer/Vulkan/VulkanDevice.hxx"
#include "Engine/Renderer/Vulkan/VulkanResources.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogLinuxApplication, Warn)

Application *GApplication = nullptr;

LinuxApplication::LinuxApplication()
{
    RHI = Ref<VulkanRHI::VulkanDynamicRHI>::Create();

    check(GApplication == nullptr);
    GApplication = this;
}

LinuxApplication::~LinuxApplication()
{
    RHI = nullptr;
    GApplication = nullptr;
}

bool LinuxApplication::Initialize()
{
    GenericApplication::Initialize();

    RHI->Init();

    WindowDefinition WindowDef{
        .AppearsInTaskbar = true,
        .Title = "Raphael Engine",
    };
    Windows.push_back(Ref<LinuxWindow>::Create());
    Windows[0]->Initialize(WindowDef, nullptr);
    Windows[0]->Show();

    Viewport =
        Ref<VulkanRHI::VulkanViewport>::Create(RHI->GetDevice(), Windows[0]->GetHandle(), glm::uvec2{500u, 500u});
    Viewport->SetName("Main viewport");
    return true;
}

void LinuxApplication::Shutdown()
{
    Viewport = nullptr;

    for (Ref<LinuxWindow> &Win: Windows) {
        Win->Destroy();
    }
    Windows.clear();

    RHI->Shutdown();
    GenericApplication::Shutdown();
}

void LinuxApplication::ProcessEvent(SDL_Event SDLEvent)
{
    Ref<LinuxWindow> EventWindow = FindEventWindow(SDLEvent);
    if (!EventWindow) { return; }

    switch (SDLEvent.type) {
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED: bShouldExit = true; break;
        default: break;
    }
}

void LinuxApplication::Tick(const float DeltaTime)
{
    (void)DeltaTime;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        Ref<VulkanRHI::VulkanCmdBuffer> CommandBuffer = RHI->GetDevice()->GetCommandbuffer();
        CommandBuffer->Begin();

        ProcessEvent(event);

        CommandBuffer->End();
    }
}

bool LinuxApplication::ShouldExit() const
{
    return bShouldExit;
}

Ref<LinuxWindow> LinuxApplication::FindEventWindow(SDL_Event &Event)
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

    for (Ref<LinuxWindow> &Window: Windows) {
        if (SDL_GetWindowID(Window->GetHandle()) == WindowID) { return Window; }
    }
    return nullptr;
}
