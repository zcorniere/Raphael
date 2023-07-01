#include "Engine/Core/Application.hxx"

#include "Application.hxx"
#include "Engine/Core/RHI/RHI.hxx"

bool BaseApplication::OnEngineInitialization()
{
    WindowDefinition WindowDef{
        .AppearsInTaskbar = true,
        .Title = "Raphael Engine",
    };
    MainWindow = Windows.Emplace(Ref<Window>::CreateNamed("Main Window"));
    MainWindow->Initialize(WindowDef, nullptr);
    MainWindow->Show();
    MainWindow->Maximize();

    MainViewport = RHI::CreateViewport((void*)MainWindow->GetHandle(), glm::uvec2{500u, 500u});
    MainViewport->SetName("Main viewport");
    return true;
}

void BaseApplication::OnEngineDestruction()
{
    MainViewport = nullptr;
    MainWindow->Destroy();

    Windows.Clear();
}

void BaseApplication::ProcessEvent(const WindowEvent& Event)
{
    if (Event.SourceWindow != MainWindow)
    {
        return;
    }
    switch (Event.Type) {
        case WindowEvent::EventType::Close:
            bShouldExit = true;
            break;
            case WindowEvent::EventType::Resize:
            MainViewport->ResizeViewport(Event.Event.Resize.Width, Event.Event.Resize.Height);
            break;
        default:
            break;
    }
}

Ref<Window> BaseApplication::CreateNewWindow(const std::string& Name)
{
    return Windows.Emplace(Ref<Window>::CreateNamed(Name));
}

void BaseApplication::Tick(const float DeltaTime)
{
    (void)DeltaTime;

    // Process All event
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        WindowEvent ConvertedEvent = ConvertWindowEvent(event);
        ProcessEvent(ConvertedEvent);
    }
}

bool BaseApplication::ShouldExit() const
{
    return bShouldExit;
}

WindowEvent BaseApplication::ConvertWindowEvent(const SDL_Event& Event)
{
    uint32 WindowID;

    WindowEvent ConvertedEvent{
        .Type = WindowEvent::EventType::Unknown,
        .SourceWindow = nullptr,
    };

    switch (Event.type) {
        case SDL_EVENT_TEXT_INPUT:
            WindowID = Event.text.windowID;
            break;
        case SDL_EVENT_TEXT_EDITING:
            WindowID = Event.edit.windowID;
            break;
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            WindowID = Event.key.windowID;
            break;
        case SDL_EVENT_MOUSE_MOTION:
            WindowID = Event.motion.windowID;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            WindowID = Event.button.windowID;
            break;
        case SDL_EVENT_MOUSE_WHEEL:
            WindowID = Event.wheel.windowID;
            break;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            WindowID = Event.window.windowID;
            ConvertedEvent.Type = WindowEvent::EventType::Close;
            ConvertedEvent.Event.Close = EventClose{};
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            WindowID = Event.window.windowID;
            ConvertedEvent.Type = WindowEvent::EventType::Resize;
            ConvertedEvent.Event.Resize = EventResize{
                .Width = static_cast<uint32>(Event.window.data2),
                .Height = static_cast<uint32>(Event.window.data1),
            };
            break;
        case SDL_EVENT_WINDOW_MOUSE_ENTER:
        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
        case SDL_EVENT_WINDOW_FOCUS_LOST:
        case SDL_EVENT_WINDOW_TAKE_FOCUS:
        case SDL_EVENT_WINDOW_HIT_TEST:
            WindowID = Event.window.windowID;
            break;
        default:
            break;
    }

    for (Ref<Window>& Window: Windows) {
        if (SDL_GetWindowID(Window->GetHandle()) == WindowID) {
            ConvertedEvent.SourceWindow = Window;
            break;
        }
    }
    return ConvertedEvent;
}
