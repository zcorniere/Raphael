#pragma once

#include <functional>

enum class EEventType {
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

enum EEventCategory {
    None = 0,
    EventCategoryApplication = BIT(0),
    EventCategoryInput = BIT(1),
    EventCategoryKeyboard = BIT(2),
    EventCategoryMouse = BIT(3),
    EventCategoryMouseButton = BIT(4),
};

#define EVENT_CLASS_TYPE(Type)                       \
    static EEventType GetStaticType()                \
    {                                                \
        return EEventType::Type;                     \
    }                                                \
    virtual EEventType GetEventType() const override \
    {                                                \
        return GetStaticType();                      \
    }                                                \
    virtual const char* GetName() const override     \
    {                                                \
        return #Type;                                \
    }

#define EVENT_CLASS_CATEGORY(category)            \
    virtual int GetCategoryFlags() const override \
    {                                             \
        return category;                          \
    }

class FEvent : public RTTI::FEnable
{
    RTTI_DECLARE_TYPEINFO(FEvent);

public:
    virtual ~FEvent()
    {
    }
    virtual EEventType GetEventType() const = 0;
    virtual const char* GetName() const = 0;
    virtual int GetCategoryFlags() const = 0;

    inline bool IsInCategory(EEventCategory category)
    {
        return GetCategoryFlags() & category;
    }

public:
    bool Handled = false;
};

template <typename T>
concept DispatchableEvent = std::derived_from<T, FEvent> && requires(T a) {
    { T::GetStaticType() } -> std::convertible_to<EEventType>;
};

class FEventDispatcher
{
private:
    template <DispatchableEvent T>
    using EventFn = std::function<bool(T&)>;

public:
    FEventDispatcher(FEvent& event): m_Event(event)
    {
    }

    template <DispatchableEvent T>
    bool Dispatch(EventFn<T>&& func)
    {
        if (m_Event.GetEventType() == T::GetStaticType() && !m_Event.Handled) {
            T* const CastedEvent = m_Event.Cast<T>();
            check(CastedEvent);
            m_Event.Handled = func(*CastedEvent);
            return true;
        }
        return false;
    }

private:
    FEvent& m_Event;
};

DEFINE_PRINTABLE_TYPE(FEvent, "Event {{ Name: {:s}, Type: {:s} }}", Value.GetName(),
                      magic_enum::enum_name(Value.GetEventType()));
