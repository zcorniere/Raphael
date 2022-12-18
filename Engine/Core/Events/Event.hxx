#pragma once

#include "Engine/Misc/MiscDefines.hxx"

#include <functional>

namespace Raphael
{

enum class EventType {
    None = 0,

    WindowClose,
    WindowMinimize,
    WindowResize,
    WindowFocus,
    WindowLostFocus,
    WindowsMoved,

    KeyPressed,
    KeyReleased,
    KeyTyped,

    MouseButtonPressed,
    MouseButtonReleased,
    MouseMoved,
    MouseScrolled,
};

enum EventCategory {
    None = 0,
    EventCategoryApplication = BIT(0),
    EventCategoryInput = BIT(1),
    EventCategoryKeyboard = BIT(2),
    EventCategoryMouse = BIT(3),
    EventCategoryMouseButton = BIT(4),
    EventCategoryScene = BIT(5),
    EventCategoryEditor = BIT(6)
};

#define EVENT_CLASS_TYPE(Type)                              \
    static EventType GetStaticType()                        \
    {                                                       \
        return EventType::Type;                             \
    }                                                       \
    virtual EventType GetEventType() const override         \
    {                                                       \
        return GetStaticType();                             \
    }                                                       \
    virtual const std::string_view GetName() const override \
    {                                                       \
        return #Type;                                       \
    }

#define EVENT_CLASS_CATEGORY(Category)            \
    virtual int GetCategoryFlags() const override \
    {                                             \
        return Category;                          \
    }

class Event
{
public:
    virtual ~Event(){};
    virtual EventType GetEventType() const = 0;
    virtual const std::string_view GetName() const = 0;
    virtual int GetCategoryFlags() const = 0;
    virtual std::string ToString() const
    {
        return std::string(GetName());
    }

    bool IsInCategory(EventCategory Category)
    {
        return GetCategoryFlags() & Category;
    }

public:
    bool Handled = false;
};

class EventDispatcher
{
private:
    template <typename T>
    using EventFn = std::function<bool(T &)>;

public:
    EventDispatcher(Event &Event): m_Event(Event)
    {
    }

    template <typename T>
    requires std::is_base_of_v<Event, T>
    bool Dispatch(EventFn<T> Func)
    {
        if (m_Event.GetEventType() == T::GetStaticType() && !m_Event.Handled) {
            m_Event.Handled = Func(static_cast<T &>(m_Event));
            return true;
        }
        return false;
    }

private:
    Event &m_Event;
};

inline std::ostream &operator<<(std::ostream &os, const Event &e)
{
    return os << e.ToString();
}

}    // namespace Raphael
