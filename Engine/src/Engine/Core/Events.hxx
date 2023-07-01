#pragma once

#include "Engine/Core/Window.hxx"

struct EventClose {
};
struct EventResize {
    uint32 Width;
    uint32 Height;
};

struct WindowEvent {
    enum class EventType {
        Unknown,
        Close,
        Resize,
        Maximize,
        Minimize,
    } Type;

    Ref<Window> SourceWindow;

    union Event {
        EventClose Close;
        EventResize Resize;
    } Event;
};
