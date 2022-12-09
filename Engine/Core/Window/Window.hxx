#pragma once

#include "Engine/Core/Events/Event.hxx"
#include "Engine/Core/Ref.hxx"

namespace Raphael
{

struct WindowSpecification {
    std::string Title = "Raphael";
    std::uint32_t Width = 1600;
    std::uint32_t Height = 900;
    bool Decorated = true;
    bool Fullscreen = false;
    bool VSync = true;
};

class Window : public RefCounted
{
public:
    using EventCallbackFn = std::function<void(Event &)>;

public:
    virtual ~Window(){};

    virtual void Init() = 0;
    virtual void ProcessEvents() = 0;
    virtual void SwapBuffers() = 0;

    virtual std::uint32_t GetWidth() const = 0;
    virtual std::uint32_t GetHeight() const = 0;
    virtual std::pair<std::uint32_t, std::uint32_t> GetSize() const = 0;
    virtual std::pair<float, float> GetWindowPos() const = 0;

    virtual void Maximize() = 0;
    virtual void CenterWindow() = 0;

    virtual void SetEventCallback(const EventCallbackFn &callback) = 0;
    virtual void SetVSync(bool enabled) = 0;
    virtual bool IsVSync() const = 0;
    virtual void SetResizable(bool resizable) const = 0;

    virtual const std::string &GetTitle() const = 0;
    virtual void SetTitle(const std::string &Title) = 0;

    virtual void *GetNativeWindow() const = 0;

    static Window *Create(const WindowSpecification &spec = WindowSpecification());
};

}    // namespace Raphael
