#pragma once

#include "Engine/Core/Window/Window.hxx"

#include <GLFW/glfw3.h>

namespace Raphael
{

class UnixWindow : public Window
{
public:
    UnixWindow(const WindowSpecification &specification);
    virtual ~UnixWindow();

    virtual void Init() override;
    virtual void ProcessEvents() override;
    virtual void SwapBuffers() override;

    virtual std::uint32_t GetWidth() const override
    {
        return m_Data.Width;
    }
    virtual std::uint32_t GetHeight() const override
    {
        return m_Data.Height;
    }
    virtual std::pair<std::uint32_t, std::uint32_t> GetSize() const override
    {
        return {m_Data.Width, m_Data.Height};
    }
    virtual std::pair<float, float> GetWindowPos() const override;

    virtual void Maximize() override;
    virtual void CenterWindow() override;

    virtual void SetEventCallback(const EventCallbackFn &callback) override
    {
        m_Data.EventCallback = callback;
    }
    virtual void SetVSync(bool enabled) override;
    virtual bool IsVSync() const override;
    virtual void SetResizable(bool resizable) const override;

    virtual const std::string &GetTitle() const override
    {
        return m_Data.Title;
    }
    virtual void SetTitle(const std::string &Title) override;

    virtual void *GetNativeWindow() const override
    {
        return m_Window;
    }

private:
    virtual void Shutdown();

private:
    GLFWwindow *m_Window = nullptr;
    WindowSpecification m_Specification;

    struct WindowData {
        std::string Title;
        std::uint32_t Width;
        std::uint32_t Height;

        EventCallbackFn EventCallback;
    };
    WindowData m_Data;
};

}    // namespace Raphael
