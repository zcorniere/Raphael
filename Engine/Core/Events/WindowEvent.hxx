#pragma once

#include <sstream>

#include "Engine/Core/Events/Event.hxx"
#include "Engine/Core/Window/KeyCodes.hxx"

namespace Raphael
{

class WindowResizeEvent : public Event
{
public:
    WindowResizeEvent(unsigned int width, unsigned int height): m_Width(width), m_Height(height)
    {
    }

    inline unsigned int GetWidth() const
    {
        return m_Width;
    }
    inline unsigned int GetHeight() const
    {
        return m_Height;
    }

    std::string ToString() const override
    {
        std::stringstream ss;
        ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
        return ss.str();
    }

    EVENT_CLASS_TYPE(WindowResize)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
private:
    unsigned int m_Width, m_Height;
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

}    // namespace Raphael
