#pragma once

#include "Engine/Core/Events/Events.hxx"

class WindowResizeEvent : public Event
{
public:
    WindowResizeEvent(uint32 width, uint32 height): m_Width(width), m_Height(height)
    {
    }

    inline uint32 GetWidth() const
    {
        return m_Width;
    }
    inline uint32 GetHeight() const
    {
        return m_Height;
    }

    EVENT_CLASS_TYPE(WindowResize)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
private:
    uint32 m_Width;
    uint32_t m_Height;
};

class WindowMinimizeEvent : public Event
{
public:
    WindowMinimizeEvent(bool minimized): m_Minimized(minimized)
    {
    }

    bool IsMinimized() const
    {
        return m_Minimized;
    }

    EVENT_CLASS_TYPE(WindowMinimize)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
private:
    bool m_Minimized = false;
};

class WindowCloseEvent : public Event
{
public:
    WindowCloseEvent()
    {
    }

    EVENT_CLASS_TYPE(WindowClose)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

DEFINE_PRINTABLE_TYPE(WindowResizeEvent, "WindowResizeEvent {{ Width: {}, Height: {} }}", Value.GetWidth(),
                      Value.GetHeight());
DEFINE_PRINTABLE_TYPE(WindowMinimizeEvent, "WindowMinimizeEvent {{ IsMinimized: {} }}", Value.IsMinimized());
