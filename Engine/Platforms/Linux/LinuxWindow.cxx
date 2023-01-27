#include "Engine/Platforms/Linux/LinuxWindow.hxx"

#include "Engine/Misc/MiscDefines.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogLinuxWindow, Info);

bool GSDLInitialized = false;

uint32 GWindowStyleSDL = SDL_WINDOW_VULKAN;

namespace Raphael::Windows
{

void LinuxWindow::EnsureSDLInit()
{
    if (!InitializeSDL()) {
        LOG(LogLinuxWindow, Fatal, "LinuxWindow::Initialize() : Failed to initialize window");
        checkNoEntry();
        return;
    }
}

LinuxWindow::LinuxWindow(): p_HWnd(nullptr), bIsVisible(false), p_ParentWindow(nullptr)
{
}

LinuxWindow::~LinuxWindow()
{
}

void LinuxWindow::Initialize(const std::shared_ptr<WindowDefinition> &InDefinition, const Ref<LinuxWindow> &InParent)
{
    Definition = InDefinition;
    p_ParentWindow = InParent;

    EnsureSDLInit();

    const float XInitialRect = Definition->XPositionOnScreen;
    const float YInitialRect = Definition->YPositionOnScreen;

    const float WidthInitial = Definition->WidthDesiredOnScreen;
    const float HeightInitial = Definition->HeightDesiredOnScreen;

    int32 X = XInitialRect;
    int32 Y = YInitialRect;
    int32 Width = WidthInitial;
    int32 Height = HeightInitial;

    uint32 WindowStyle = GWindowStyleSDL;
    if (!Definition->HasOsWindowBorder) {
        WindowStyle |= SDL_WINDOW_BORDERLESS;
        if (!Definition->AppearsInTaskbar) { WindowStyle |= SDL_WINDOW_SKIP_TASKBAR; }
    }
    if (Definition->IsRegularWindow && Definition->HasSizingFrame) { WindowStyle |= SDL_WINDOW_RESIZABLE; }

    p_HWnd = SDL_CreateWindow(Definition->Title.c_str(), X, Y, Width, Height, WindowStyle);
    if (!p_HWnd) {
        LOG(LogLinuxWindow, Fatal, "Failed To create the SDL Window");
        checkNoEntry();
        return;
    }

    if (WindowStyle & SDL_WINDOW_RESIZABLE) { SDL_SetWindowMinimumSize(p_HWnd, 100, 100); }
    SDL_SetWindowModalFor(p_HWnd, p_ParentWindow->GetHandle());
}

const Ref<LinuxWindow> &LinuxWindow::GetParent() const
{
    return p_ParentWindow;
}

void LinuxWindow::ReshapeWindow(int32 X, int32 Y, int32 Width, int32 Height)
{
    (void)X;
    (void)Y;
    (void)Width;
    (void)Height;
}

void LinuxWindow::MoveWindow(int32 X, int32 Y)
{
    SDL_SetWindowPosition(p_HWnd, X, Y);
}

void LinuxWindow::BringToFront(bool bForce)
{
    if (bForce) {
        SDL_RaiseWindow(p_HWnd);
    } else {
        Show();
    }
}

void LinuxWindow::Destroy()
{
    if (p_HWnd) {

        LOG(LogLinuxWindow, Info, "Destroying SDL Window '{:p}'", (void *)p_HWnd);
        SDL_DestroyWindow(p_HWnd);
        p_HWnd = nullptr;
    }
}

void LinuxWindow::Minimize()
{
    SDL_MinimizeWindow(p_HWnd);
}

void LinuxWindow::Maximize()
{
    SDL_MaximizeWindow(p_HWnd);
}

void LinuxWindow::Restore()
{
    SDL_RestoreWindow(p_HWnd);
}
void LinuxWindow::Show()
{
    if (IsMinimized()) { Restore(); }

    if (!bIsVisible) {
        bIsVisible = true;
        SDL_ShowWindow(p_HWnd);
    }
}

void LinuxWindow::Hide()
{
    if (bIsVisible) {
        bIsVisible = false;
        SDL_HideWindow(p_HWnd);
    }
}

bool LinuxWindow::IsMaximized() const
{
    return SDL_GetWindowFlags(p_HWnd) & SDL_WINDOW_MAXIMIZED;
}

bool LinuxWindow::IsMinimized() const
{
    return SDL_GetWindowFlags(p_HWnd) & SDL_WINDOW_MINIMIZED || SDL_GetWindowFlags(p_HWnd) & SDL_WINDOW_HIDDEN;
}

bool LinuxWindow::IsVisible() const
{
    return bIsVisible;
}

void LinuxWindow::AcceptInput(bool bEnable)
{
    (void)bEnable;
}

int32 LinuxWindow::GetWindowBorderSize() const
{
    return 0;
}

int32 LinuxWindow::GetWindowTitleBarSize() const
{
    return 0;
}

void LinuxWindow::SetText(const std::string_view Text)
{
    SDL_SetWindowTitle(p_HWnd, Text.data());
}

void LinuxWindow::DrawAttention(bool bStop)
{
    if (bStop) {
        SDL_FlashWindow(p_HWnd, SDL_FLASH_CANCEL);
    } else {
        SDL_FlashWindow(p_HWnd, SDL_FLASH_UNTIL_FOCUSED);
    }
}

SDL_Window *LinuxWindow::GetHandle()
{
    return p_HWnd;
}

bool LinuxWindow::InitializeSDL()
{
    if (GSDLInitialized) { return true; }
    LOG(LogLinuxWindow, Info, "Initializing SDL.");

    SDL_SetHint("SDL_VIDEO_X11_REQUIRE_XRANDR", "1");

    if (SDL_Init((SDL_INIT_EVERYTHING ^ SDL_INIT_AUDIO)) != 0) {
        const char *ErrorMessage = SDL_GetError();
        if (strcmp("No message system available", ErrorMessage) != 0) {
            LOG(LogLinuxWindow, Warn, "Could not initialize SDL: {}", ErrorMessage);
        }
    }

    SDL_version CompileTimeSDLVersion;
    SDL_version RunTimeSDLVersion;
    SDL_VERSION(&CompileTimeSDLVersion);
    SDL_GetVersion(&RunTimeSDLVersion);
    const char *SdlRevision = SDL_GetRevision();

    LOG(LogLinuxWindow, Info, "Initialized GLFW {:d}.{:d}.{:d} revision {} (compiled against {:d}.{:d}.{:d})",
        RunTimeSDLVersion.major, RunTimeSDLVersion.minor, RunTimeSDLVersion.patch, SdlRevision,
        CompileTimeSDLVersion.major, CompileTimeSDLVersion.minor, CompileTimeSDLVersion.patch);
    char const *SdlVideoDriver = SDL_GetCurrentVideoDriver();
    if (SdlVideoDriver) { LOG(LogLinuxWindow, Info, "Using SDL video driver '{}'", SdlVideoDriver); }

    GSDLInitialized = true;
    return true;
}

}    // namespace Raphael::Windows
