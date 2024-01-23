#pragma once

#include "Engine/Core/Events/Events.hxx"
#include "Engine/Core/Events/KeyCodes.hxx"

class MouseMovedEvent : public Event
{
public:
    MouseMovedEvent(float x, float y): m_MouseX(x), m_MouseY(y)
    {
    }

    inline float GetX() const
    {
        return m_MouseX;
    }
    inline float GetY() const
    {
        return m_MouseY;
    }

    EVENT_CLASS_TYPE(MouseMoved)
    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
private:
    float m_MouseX, m_MouseY;
};

class MouseScrolledEvent : public Event
{
public:
    MouseScrolledEvent(float xOffset, float yOffset): m_XOffset(xOffset), m_YOffset(yOffset)
    {
    }

    inline float GetXOffset() const
    {
        return m_XOffset;
    }
    inline float GetYOffset() const
    {
        return m_YOffset;
    }

    EVENT_CLASS_TYPE(MouseScrolled)
    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
private:
    float m_XOffset, m_YOffset;
};

class MouseButtonEvent : public Event
{
public:
    inline MouseButton GetMouseButton() const
    {
        return m_Button;
    }

    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
protected:
    MouseButtonEvent(MouseButton button): m_Button(button)
    {
    }

    MouseButton m_Button;
};

class MouseButtonPressedEvent : public MouseButtonEvent
{
public:
    MouseButtonPressedEvent(MouseButton button): MouseButtonEvent(button)
    {
    }

    EVENT_CLASS_TYPE(MouseButtonPressed)
};

class MouseButtonReleasedEvent : public MouseButtonEvent
{
public:
    MouseButtonReleasedEvent(MouseButton button): MouseButtonEvent(button)
    {
    }

    EVENT_CLASS_TYPE(MouseButtonReleased)
};

class MouseButtonDownEvent : public MouseButtonEvent
{
public:
    MouseButtonDownEvent(MouseButton button): MouseButtonEvent(button)
    {
    }

    EVENT_CLASS_TYPE(MouseButtonDown)
};

DEFINE_PRINTABLE_TYPE(MouseMovedEvent, "MouseMovedEvent {{ X: {}, Y: {} }}", Value.GetX(), Value.GetY());
DEFINE_PRINTABLE_TYPE(MouseScrolledEvent, "MouseScrolledEvent {{ OffsetX: {}, OffsetY: {} }}", Value.GetXOffset(),
                      Value.GetYOffset());
DEFINE_PRINTABLE_TYPE(MouseButtonPressedEvent, "MouseButtonPressedEvent {{ Button: {} }}", Value.GetMouseButton())
DEFINE_PRINTABLE_TYPE(MouseButtonReleasedEvent, "MouseButtonReleasedEvent {{ Button: {} }}", Value.GetMouseButton())
DEFINE_PRINTABLE_TYPE(MouseButtonDownEvent, "MouseButtonDownEvent {{ Button: {} }}", Value.GetMouseButton())
