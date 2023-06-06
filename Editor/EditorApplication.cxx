#include "EditorApplication.hxx"

#include <Engine/Core/FrameGraph/FrameGraph.hxx>

DECLARE_LOGGER_CATEGORY(Core, LogApplication, Warning)

EditorApplication::EditorApplication()
{
}

EditorApplication::~EditorApplication()
{
}

bool EditorApplication::OnEngineInitialization()
{
    WindowDefinition WindowDef{
        .AppearsInTaskbar = true,
        .Title = "Raphael Engine",
    };
    Windows.push_back(Ref<Window>::CreateNamed("Main Window"));
    Windows[0]->Initialize(WindowDef, nullptr);
    Windows[0]->Show();
    Windows[0]->Maximize();

    Viewport = RHI::CreateViewport((void*)Windows[0]->GetHandle(), glm::uvec2{500u, 500u});
    Viewport->SetName("Main viewport");

#if 0
    struct FrameData {
        FrameGraphResource Texture;
        FrameGraphResource Shader;
    };
    FrameGraph Graph;
    Graph.AddCallbackPass<FrameData>(
        "Test pass",
        [](FrameGraphBuilder &Builder, FrameData &Data) {
            RHITextureCreateDesc Description{};
            Data.Texture = Builder.Create<RHIResourceType::Texture>("Test Texture", Description);
            Data.Shader = Builder.Create<RHIResourceType::Shader>("Triangle shader",
                                                                  std::filesystem::path("DefaultTriangle.vert"), false);

            LOG(LogApplication, Info, "Setup");
        },
        [](const FrameData &, FrameGraphPassResources &) { LOG(LogApplication, Info, "Execution"); });
    Graph.Compile();
    Graph.Execute();
#endif

    // Create a shader to test the reflection
    RHI::CreateShader("DefaultTriangle.vert", true);
    RHI::CreateShader("DefaultTriangle.frag", true);

    return true;
}

void EditorApplication::OnEngineDestruction()
{
    Viewport = nullptr;

    for (Ref<Window>& Win: Windows) {
        Win->Destroy();
    }
    Windows.clear();
}

void EditorApplication::ProcessEvent(SDL_Event SDLEvent)
{
    Ref<Window> EventWindow = FindEventWindow(SDLEvent);
    if (!EventWindow) {
        return;
    }

    switch (SDLEvent.type) {
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            bShouldExit = true;
            break;
        default:
            break;
    }
}

void EditorApplication::Tick(const float DeltaTime)
{
    (void)DeltaTime;
    SDL_Event event;
    // Process All event
    while (SDL_PollEvent(&event)) {
        ProcessEvent(event);
    }

    Viewport->BeginDrawViewport();
    Viewport->EndDrawViewport();
}

bool EditorApplication::ShouldExit() const
{
    return bShouldExit;
}

Ref<Window> EditorApplication::FindEventWindow(SDL_Event& Event)
{
    uint32 WindowID;

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
        case SDL_EVENT_WINDOW_MOUSE_ENTER:
        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
        case SDL_EVENT_WINDOW_FOCUS_LOST:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        case SDL_EVENT_WINDOW_TAKE_FOCUS:
        case SDL_EVENT_WINDOW_HIT_TEST:
            WindowID = Event.window.windowID;
            break;
        default:
            return nullptr;
    }

    for (Ref<Window>& Window: Windows) {
        if (SDL_GetWindowID(Window->GetHandle()) == WindowID) {
            return Window;
        }
    }
    return nullptr;
}
