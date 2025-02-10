#pragma once

#include "Engine/Core/Events/Events.hxx"

struct GLFWwindow;

/// @brief Define a Window
struct FWindowDefinition {
    using EventHandler = std::function<void(FEvent&)>;

    /// The X position on the screen
    float XPositionOnScreen = 0;
    /// The Y position on the screen
    float YPositionOnScreen = 0;

    /// Width of the window
    /// @note The window could end up smaller if it does not fit on screen
    float WidthDesiredOnScreen = 500;
    /// Height of the window
    /// @note The window could end up smaller if it does not fit on screen
    float HeightDesiredOnScreen = 500;

    /// Should the window as OS border ?
    bool HasOsWindowBorder = true;
    /// Should the window appears in the Taskbar ?
    bool AppearsInTaskbar = true;
    /// Should the window accept input ?
    bool AcceptInput = true;
    /// Should the window be resizable ?
    bool HasSizingFrame = true;

    /// The title of the window
    std::string Title = __FILE__;
    /// Should the resize preserve aspect ratio
    bool ShouldPreserveAspectRatio = true;

    EventHandler EventCallback;
};

/// @class Window
///
/// @brief A class allowing some abstraction over the GLFW library
class RWindow : public RObject
{
    RTTI_DECLARE_TYPEINFO(RWindow, RObject);

public:
    /// Make sure GLFW is initialized or do it if it is not
    static bool EnsureGLFWInit();

public:
    /// Default ctor
    RWindow();
    /// Default dtor
    virtual ~RWindow();

    /// @brief Open the window
    /// @param InDefinition The definition of the window
    /// @param InParent (optional) The parent window of this window
    void Initialize(const FWindowDefinition& InDefinition);

    /// Reshape the window
    void ReshapeWindow(int32 X, int32 Y, int32 Width, int32 Height);
    /// Move the window
    void MoveWindow(int32 X, int32 Y);
    /// Bring the window to the front
    void BringToFront(bool bForce = false);

    /// Destroy the window
    void Destroy();
    /// Minimize the window
    void Minimize();
    /// Maximize the window
    void Maximize();
    /// Restore the window
    void Restore();

    /// @brief Show the window
    void Show();
    /// @brief Hide the window
    void Hide();

    bool IsMaximized() const;
    bool IsMinimized() const;
    bool IsVisible() const;

    void AcceptInput(bool bEnable);
    int32 GetWindowBorderSize() const;
    int32 GetWindowTitleBarSize() const;

    void SetText(const std::string_view Text);
    void DrawAttention();

    GLFWwindow* GetHandle() const;

    const FWindowDefinition& GetDefinition() const
    {
        return Definition;
    }

    void ProcessEvents();

private:
    static std::atomic_bool bGLFWInitialized;
    static std::atomic_short GFLWInUseCount;
    static bool InitializeGLFW();

    void SetupGLFWCallbacks();

private:
    FWindowDefinition Definition;
    GLFWwindow* p_Handle;

    bool bIsVisible;
};
