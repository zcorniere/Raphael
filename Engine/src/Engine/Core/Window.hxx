#pragma once

#include <SDL3/SDL.h>

/// @brief Define a Window
struct WindowDefinition {
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
    /// Should the resive preserve aspect ratio
    bool ShouldPreserveAspectRatio;
};

/// @class Window
///
/// @brief A class allowing some abstaction over the SDL library
///
/// @code{.cpp}
/// WindowDefinition WindowDef{
///     .AppearsInTaskbar = true,
///     .Title = "Raphael Engine",
/// };
/// // Declare and create a new window
/// Ref<Window> window = Ref<Window>::Create();
///
/// window->Initialize(WindowDef);
/// window->Show();
/// @endcode
class Window : public RObject
{
public:
    /// Make sure the SDL is initialized or do it if it is not
    static bool EnsureSDLInit();

public:
    /// Default ctor
    Window();
    /// Default dtor
    ~Window();

    /// @brief Open the window
    /// @param InDefinition The definition of the window
    /// @param InParent (optional) The parent window of this window
    void Initialize(const WindowDefinition InDefinition, const Ref<Window>& InParent = nullptr);

    /// Return the parent window, or nullptr
    const Ref<Window>& GetParent() const;

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
    void DrawAttention(bool bStop = false);

    SDL_Window* GetHandle();

    const WindowDefinition& GetDefinition() const
    {
        return Definition;
    }

private:
    static bool InitializeSDL();

private:
    WindowDefinition Definition;
    SDL_Window* p_HWnd;

    bool bIsVisible;
    Ref<Window> p_ParentWindow;
};
