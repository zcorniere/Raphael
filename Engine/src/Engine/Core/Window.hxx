#pragma once

#include <SDL.h>

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

class Window : public RObject
{
public:
    static bool EnsureSDLInit();

public:
    Window();
    ~Window();

    void Initialize(const WindowDefinition InDefinition, const Ref<Window>& InParent);

    const Ref<Window>& GetParent() const;

    void ReshapeWindow(int32 X, int32 Y, int32 Width, int32 Height);
    void MoveWindow(int32 X, int32 Y);
    void BringToFront(bool bForce = false);

    void Destroy();
    void Minimize();
    void Maximize();
    void Restore();

    void Show();
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
