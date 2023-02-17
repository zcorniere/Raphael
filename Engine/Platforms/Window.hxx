#pragma once

#include "Engine/Core/RObject.hxx"
#include "Engine/Platforms/Platform.hxx"

struct WindowDefinition {
    float XPositionOnScreen = 0;
    float YPositionOnScreen = 0;

    float WidthDesiredOnScreen = 500;
    float HeightDesiredOnScreen = 500;

    bool HasOsWindowBorder = true;
    bool AppearsInTaskbar = true;
    bool AcceptInput;
    bool IsRegularWindow = true;
    bool HasSizingFrame = true;

    std::string Title = __FILE__;
    bool ShouldPreserveAspectRatio;
};

class GenericWindow : public RObject
{

public:
    virtual ~GenericWindow()
    {
    }

    virtual void ReshapeWindow([[maybe_unused]] int32 X, [[maybe_unused]] int32 Y, [[maybe_unused]] int32 Width,
                               [[maybe_unused]] int32 Height)
    {
    }
    virtual void MoveWindow([[maybe_unused]] int32 X, [[maybe_unused]] int32 Y)
    {
    }
    virtual void BringToFront([[maybe_unused]] bool bForce = false)
    {
    }

    virtual void Destroy()
    {
    }
    virtual void Minimize()
    {
    }
    virtual void Maximize()
    {
    }
    virtual void Restore()
    {
    }

    virtual void Show()
    {
    }
    virtual void Hide()
    {
    }

    virtual bool IsMaximized() const
    {
        return true;
    }
    virtual bool IsMinimized() const
    {
        return true;
    }
    virtual bool IsVisible() const
    {
        return true;
    }

    virtual void AcceptInput([[maybe_unused]] bool bEnable)
    {
    }

    virtual int32 GetWindowBorderSize() const
    {
        return 0;
    }
    virtual int32 GetWindowTitleBarSize() const
    {
        return 0;
    }

    virtual void SetText([[maybe_unused]] const std::string_view Text)
    {
    }

    virtual void DrawAttention([[maybe_unused]] bool bStop = false)
    {
    }

    const WindowDefinition &GetDefinition() const
    {
        return *Definition;
    }

protected:
    std::shared_ptr<WindowDefinition> Definition;
};

#if defined(PLATFORM_WINDOWS)
    #include "Engine/Platforms/Windows/WindowsWindow.hxx"
#elif defined(PLATFORM_LINUX)
    #include "Engine/Platforms/Linux/LinuxWindow.hxx"
#else
static_assert(false, "Unsuported Platform !");
#endif
