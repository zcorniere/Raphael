#pragma once

#include <functional>

enum class EventType {
    None = 0,
    WindowClose,
    WindowMinimize,
    WindowResize,
    WindowFocus,
    WindowLostFocus,
    WindowMoved,
    KeyPressed,
    KeyReleased,
    KeyTyped,
    MouseButtonPressed,
    MouseButtonReleased,
    MouseButtonDown,
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
};

#define EVENT_CLASS_TYPE(type)                      \
    static EventType GetStaticType()                \
    {                                               \
        return EventType::##type;                   \
    }                                               \
    virtual EventType GetEventType() const override \
    {                                               \
        return GetStaticType();                     \
    }                                               \
    virtual const char* GetName() const override    \
    {                                               \
        return #type;                               \
    }

#define EVENT_CLASS_CATEGORY(category)            \
    virtual int GetCategoryFlags() const override \
    {                                             \
        return category;                          \
    }

class Event
{
public:
    virtual ~Event()
    {
    }
    virtual EventType GetEventType() const = 0;
    virtual const char* GetName() const = 0;
    virtual int GetCategoryFlags() const = 0;

    inline bool IsInCategory(EventCategory category)
    {
        return GetCategoryFlags() & category;
    }

public:
    bool Handled = false;
};

template <typename T>
concept DispatchableEvent = std::derived_from<T, Event> && requires(T a) {
    {
        T::GetStaticType()
    } -> std::convertible_to<EventType>;
};

class EventDispatcher
{
private:
    template <DispatchableEvent T>
    using EventFn = std::function<bool(T&)>;

public:
    EventDispatcher(Event& event): m_Event(event)
    {
    }

    template <DispatchableEvent T>
    bool Dispatch(EventFn<T>&& func)
    {
        if (m_Event.GetEventType() == T::GetStaticType() && !m_Event.Handled) {
            T* CastedEvent = dynamic_cast<T*>(&m_Event);
            check(CastedEvent);
            m_Event.Handled = func(*CastedEvent);
            return true;
        }
        return false;
    }

private:
    Event& m_Event;
};

DEFINE_PRINTABLE_TYPE(Event, "Event {{ Name: {:s}, Type: {:s} }}", Value.GetName(),
                      magic_enum::enum_name(Value.GetEventType()));
