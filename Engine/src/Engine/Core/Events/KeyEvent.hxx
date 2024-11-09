#pragma once

#include "Engine/Core/Events/Events.hxx"
#include "Engine/Core/Events/KeyCodes.hxx"

class FKeyEvent : public FEvent
{
public:
    inline EKeyCode GetKeyCode() const
    {
        return m_KeyCode;
    }

    EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

protected:
    FKeyEvent(EKeyCode keycode): m_KeyCode(keycode)
    {
    }

    EKeyCode m_KeyCode;
};

class FKeyPressedEvent : public FKeyEvent
{
public:
    FKeyPressedEvent(EKeyCode keycode, int repeatCount): FKeyEvent(keycode), m_RepeatCount(repeatCount)
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

class FKeyReleasedEvent : public FKeyEvent
{
public:
    FKeyReleasedEvent(EKeyCode keycode): FKeyEvent(keycode)
    {
    }

    EVENT_CLASS_TYPE(KeyReleased)
};

class FKeyTypedEvent : public FKeyEvent
{
public:
    FKeyTypedEvent(EKeyCode keycode): FKeyEvent(keycode)
    {
    }

    EVENT_CLASS_TYPE(KeyTyped)
};

DEFINE_PRINTABLE_TYPE(FKeyPressedEvent, "KeyReleasedEvent {{ Key: {:s}, RepeatCount: {} }}",
                      magic_enum::enum_name(Value.GetKeyCode()), Value.GetRepeatCount());
DEFINE_PRINTABLE_TYPE(FKeyReleasedEvent, "KeyReleasedEvent {{ Key: {:s} }}", magic_enum::enum_name(Value.GetKeyCode()));
DEFINE_PRINTABLE_TYPE(FKeyTypedEvent, "KeyTypedEvent {{ Key: {:s} }}", magic_enum::enum_name(Value.GetKeyCode()));
