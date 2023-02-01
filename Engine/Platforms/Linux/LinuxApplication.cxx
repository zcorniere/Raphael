#include "Engine/Platforms/Linux/LinuxApplication.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogLinuxApplication, Warn)

namespace Raphael
{

Ref<Application> GApplication = nullptr;

LinuxApplication::LinuxApplication()
{
    RHI = Ref<RHI::VulkanDynamicRHI>::Create();
    Windows.push_back(Ref<Windows::LinuxWindow>::Create());

    GApplication = this;
}

LinuxApplication::~LinuxApplication()
{
}

bool LinuxApplication::Initialize()
{
    check(Windows.size() >= 1);

    auto WindowDef = std::make_shared<WindowDefinition>();
    WindowDef->Title = "Raphael Engine";
    WindowDef->AppearsInTaskbar = true;
    Windows[0]->Initialize(WindowDef, nullptr);
    Windows[0]->Show();

    RHI->Init();
    return true;
}

void LinuxApplication::Shutdown()
{
    for (Ref<Windows::LinuxWindow> &Win: Windows) {
        Win->Destroy();
    }
    Windows.clear();

    RHI->Shutdown();
}

void LinuxApplication::ProcessEvent(SDL_Event SDLEvent)
{
    Ref<Windows::LinuxWindow> EventWindow = FindEventWindow(SDLEvent);
    if (!EventWindow) { return; }

    switch (SDLEvent.type) {
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED: bShouldExit = true; break;
        default: break;
    }
}

void LinuxApplication::Tick(const float DeltaTime)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ProcessEvent(event);
    }
}

bool LinuxApplication::ShouldExit() const
{
    return bShouldExit;
}

Ref<Windows::LinuxWindow> LinuxApplication::FindEventWindow(SDL_Event &Event)
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

    for (Ref<Windows::LinuxWindow> &Window: Windows) {
        if (SDL_GetWindowID(Window->GetHandle()) == WindowID) { return Window; }
    }
    return nullptr;
}

}    // namespace Raphael
