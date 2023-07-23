#pragma once

#include "Engine/Core/Events/Events.hxx"
#include "Engine/Core/Events/KeyCodes.hxx"

class KeyEvent : public Event
{
public:
    inline KeyCode GetKeyCode() const
    {
        return m_KeyCode;
    }

    EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
protected:
    KeyEvent(KeyCode keycode): m_KeyCode(keycode)
    {
    }

    KeyCode m_KeyCode;
};

class KeyPressedEvent : public KeyEvent
{
public:
    KeyPressedEvent(KeyCode keycode, int repeatCount): KeyEvent(keycode), m_RepeatCount(repeatCount)
    {
    }

    inline int GetRepeatCount() const
    {
        return m_RepeatCount;
    }

    EVENT_CLASS_TYPE(KeyPressed)
private:
    int m_RepeatCount;
};

class KeyReleasedEvent : public KeyEvent
{
public:
    KeyReleasedEvent(KeyCode keycode): KeyEvent(keycode)
    {
    }

    EVENT_CLASS_TYPE(KeyReleased)
};

class KeyTypedEvent : public KeyEvent
{
public:
    KeyTypedEvent(KeyCode keycode): KeyEvent(keycode)
    {
    }

    EVENT_CLASS_TYPE(KeyTyped)
};

DEFINE_PRINTABLE_TYPE(KeyPressedEvent, "KeyReleasedEvent {{ Key: {:s}, RepeatCount: {} }}",
                      magic_enum::enum_name(Value.GetKeyCode()), Value.GetRepeatCount());
DEFINE_PRINTABLE_TYPE(KeyReleasedEvent, "KeyReleasedEvent {{ Key: {:s} }}", magic_enum::enum_name(Value.GetKeyCode()));
DEFINE_PRINTABLE_TYPE(KeyTypedEvent, "KeyTypedEvent {{ Key: {:s} }}", magic_enum::enum_name(Value.GetKeyCode()));
