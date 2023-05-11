#include "Engine/Core/Window.hxx"

#include "Engine/Misc/MiscDefines.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogWindow, Info);

bool GSDLInitialized = false;

uint32 GWindowStyleSDL = SDL_WINDOW_VULKAN;

bool Window::EnsureSDLInit()
{
    if (!InitializeSDL()) {
        LOG(LogWindow, Fatal, "Window::Initialize() : Failed to initialize window");
        checkNoEntry();
        return false;
    }
    return true;
}

Window::Window(): p_HWnd(nullptr), bIsVisible(false), p_ParentWindow(nullptr) {}

Window::~Window() {}

void Window::Initialize(const WindowDefinition InDefinition, const Ref<Window> &InParent)
{
    Definition = InDefinition;
    p_ParentWindow = InParent;

    EnsureSDLInit();

    const float XInitialRect = Definition.XPositionOnScreen;
    const float YInitialRect = Definition.YPositionOnScreen;

    const float WidthInitial = Definition.WidthDesiredOnScreen;
    const float HeightInitial = Definition.HeightDesiredOnScreen;

    int32 X = XInitialRect;
    int32 Y = YInitialRect;
    int32 Width = WidthInitial;
    int32 Height = HeightInitial;

    uint32 WindowStyle = GWindowStyleSDL;
    if (!Definition.HasOsWindowBorder) {
        WindowStyle |= SDL_WINDOW_BORDERLESS;
        if (!Definition.AppearsInTaskbar) { WindowStyle |= SDL_WINDOW_SKIP_TASKBAR; }
    }
    if (Definition.IsRegularWindow && Definition.HasSizingFrame) { WindowStyle |= SDL_WINDOW_RESIZABLE; }

    p_HWnd = SDL_CreateWindow(Definition.Title.c_str(), X, Y, Width, Height, WindowStyle);
    if (!p_HWnd) {
        LOG(LogWindow, Fatal, "Failed To create the SDL Window");
        checkNoEntry();
        return;
    }

    if (WindowStyle & SDL_WINDOW_RESIZABLE) { SDL_SetWindowMinimumSize(p_HWnd, 100, 100); }
    if (p_ParentWindow) { SDL_SetWindowModalFor(p_HWnd, p_ParentWindow->GetHandle()); }
}

const Ref<Window> &Window::GetParent() const { return p_ParentWindow; }

void Window::ReshapeWindow(int32 X, int32 Y, int32 Width, int32 Height)
{
    (void)X;
    (void)Y;
    (void)Width;
    (void)Height;
}

void Window::MoveWindow(int32 X, int32 Y) { SDL_SetWindowPosition(p_HWnd, X, Y); }

void Window::BringToFront(bool bForce)
{
    if (bForce) {
        SDL_RaiseWindow(p_HWnd);
    } else {
        Show();
    }
}

void Window::Destroy()
{
    if (p_HWnd) {

        LOG(LogWindow, Info, "Destroying SDL Window '{:p}'", (void *)p_HWnd);
        SDL_DestroyWindow(p_HWnd);
        p_HWnd = nullptr;
    }
}

void Window::Minimize() { SDL_MinimizeWindow(p_HWnd); }

void Window::Maximize() { SDL_MaximizeWindow(p_HWnd); }

void Window::Restore() { SDL_RestoreWindow(p_HWnd); }
void Window::Show()
{
    if (IsMinimized()) { Restore(); }

    if (!bIsVisible) {
        bIsVisible = true;
        SDL_ShowWindow(p_HWnd);
    }
}

void Window::Hide()
{
    if (bIsVisible) {
        bIsVisible = false;
        SDL_HideWindow(p_HWnd);
    }
}

bool Window::IsMaximized() const { return SDL_GetWindowFlags(p_HWnd) & SDL_WINDOW_MAXIMIZED; }

bool Window::IsMinimized() const
{
    return SDL_GetWindowFlags(p_HWnd) & SDL_WINDOW_MINIMIZED || SDL_GetWindowFlags(p_HWnd) & SDL_WINDOW_HIDDEN;
}

bool Window::IsVisible() const { return bIsVisible; }

void Window::AcceptInput(bool bEnable) { (void)bEnable; }

int32 Window::GetWindowBorderSize() const { return 0; }

int32 Window::GetWindowTitleBarSize() const { return 0; }

void Window::SetText(const std::string_view Text) { SDL_SetWindowTitle(p_HWnd, Text.data()); }

void Window::DrawAttention(bool bStop)
{
    if (bStop) {
        SDL_FlashWindow(p_HWnd, SDL_FLASH_CANCEL);
    } else {
        SDL_FlashWindow(p_HWnd, SDL_FLASH_UNTIL_FOCUSED);
    }
}

SDL_Window *Window::GetHandle() { return p_HWnd; }

bool Window::InitializeSDL()
{
    if (GSDLInitialized) { return true; }
    LOG(LogWindow, Info, "Initializing SDL.");

    SDL_SetHint("SDL_VIDEO_X11_REQUIRE_XRANDR", "1");

    // Turn off the audio of SDL
    if (SDL_Init((SDL_INIT_EVERYTHING ^ SDL_INIT_AUDIO)) != 0) {
        const char *ErrorMessage = SDL_GetError();
        if (strcmp("No message system available", ErrorMessage) != 0) {
            LOG(LogWindow, Warning, "Could not initialize SDL: {}", ErrorMessage);
        }
    }

    SDL_version CompileTimeSDLVersion;
    SDL_version RunTimeSDLVersion;
    SDL_VERSION(&CompileTimeSDLVersion);
    SDL_GetVersion(&RunTimeSDLVersion);
    const char *SdlRevision = SDL_GetRevision();

    LOG(LogWindow, Info, "Initialized SDL {:d}.{:d}.{:d} revision {} (compiled against {:d}.{:d}.{:d})",
        RunTimeSDLVersion.major, RunTimeSDLVersion.minor, RunTimeSDLVersion.patch, SdlRevision,
        CompileTimeSDLVersion.major, CompileTimeSDLVersion.minor, CompileTimeSDLVersion.patch);
    char const *SdlVideoDriver = SDL_GetCurrentVideoDriver();
    if (SdlVideoDriver) { LOG(LogWindow, Info, "Using SDL video driver '{}'", SdlVideoDriver); }

    GSDLInitialized = true;
    return true;
}
