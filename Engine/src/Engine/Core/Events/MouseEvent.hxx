#pragma once

#include "Engine/Core/Events/Events.hxx"
#include "Engine/Core/Events/KeyCodes.hxx"

class FMouseMovedEvent : public FEvent
{
    RTTI_DECLARE_TYPEINFO(FMouseMovedEvent, FEvent);

public:
    FMouseMovedEvent(float x, float y): m_MouseX(x), m_MouseY(y)
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
    float m_MouseX = 0.0f;
    float m_MouseY = 0.0f;
};

class FMouseScrolledEvent : public FEvent
{
    RTTI_DECLARE_TYPEINFO(FMouseScrolledEvent, FEvent);

public:
    FMouseScrolledEvent(float xOffset, float yOffset): m_XOffset(xOffset), m_YOffset(yOffset)
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
    float m_XOffset = 0.0f;
    float m_YOffset = 0.0f;
};

class FMouseButtonEvent : public FEvent
{
    RTTI_DECLARE_TYPEINFO(FMouseButtonEvent, FEvent);

public:
    inline EMouseButton GetMouseButton() const
    {
        return m_Button;
    }

    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
protected:
    FMouseButtonEvent(EMouseButton button): m_Button(button)
    {
    }

    EMouseButton m_Button;
};

class FMouseButtonPressedEvent : public FMouseButtonEvent
{
    RTTI_DECLARE_TYPEINFO(FMouseButtonPressedEvent, FMouseButtonEvent)
public:
    FMouseButtonPressedEvent(EMouseButton button): FMouseButtonEvent(button)
    {
    }

    EVENT_CLASS_TYPE(MouseButtonPressed)
};

class FMouseButtonReleasedEvent : public FMouseButtonEvent
{
    RTTI_DECLARE_TYPEINFO(FMouseButtonReleasedEvent, FMouseButtonEvent)
public:
    FMouseButtonReleasedEvent(EMouseButton button): FMouseButtonEvent(button)
    {
    }

    EVENT_CLASS_TYPE(MouseButtonReleased)
};

class FMouseButtonDownEvent : public FMouseButtonEvent
{
    RTTI_DECLARE_TYPEINFO(FMouseButtonDownEvent, FMouseButtonEvent)
public:
    FMouseButtonDownEvent(EMouseButton button): FMouseButtonEvent(button)
    {
    }

    EVENT_CLASS_TYPE(MouseButtonDown)
};

DEFINE_PRINTABLE_TYPE(FMouseMovedEvent, "MouseMovedEvent {{ X: {}, Y: {} }}", Value.GetX(), Value.GetY());
DEFINE_PRINTABLE_TYPE(FMouseScrolledEvent, "MouseScrolledEvent {{ OffsetX: {}, OffsetY: {} }}", Value.GetXOffset(),
                      Value.GetYOffset());
DEFINE_PRINTABLE_TYPE(FMouseButtonPressedEvent, "MouseButtonPressedEvent {{ Button: {} }}", Value.GetMouseButton())
DEFINE_PRINTABLE_TYPE(FMouseButtonReleasedEvent, "MouseButtonReleasedEvent {{ Button: {} }}", Value.GetMouseButton())
DEFINE_PRINTABLE_TYPE(FMouseButtonDownEvent, "MouseButtonDownEvent {{ Button: {} }}", Value.GetMouseButton())
