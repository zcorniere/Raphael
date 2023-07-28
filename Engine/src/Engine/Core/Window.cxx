#include "Engine/Core/Window.hxx"

#include "Engine/Core/Events/ApplicationEvent.hxx"
#include "Engine/Core/Events/KeyEvent.hxx"
#include "Engine/Core/Events/MouseEvent.hxx"
#include "Engine/Misc/MiscDefines.hxx"
#include "Engine/Misc/Utils.hxx"

#include <GLFW/glfw3.h>

DECLARE_LOGGER_CATEGORY(Core, LogWindow, Info);

bool GGLFWInitialized = false;

bool Window::EnsureGLFWInit()
{
    if (!InitializeGLFW()) {
        LOG(LogWindow, Fatal, "Window::Initialize() : Failed to initialize window");
        checkNoEntry();
        return false;
    }
    return true;
}

Window::Window(): Definition(), p_Handle(nullptr), bIsVisible(false)
{
}

Window::~Window()
{
    if (p_Handle) {
        Destroy();
    }
}

void Window::Initialize(const WindowDefinition& InDefinition)
{
    Definition = InDefinition;
    checkMsg(Definition.EventCallback, "You must provide an event callback !");

    EnsureGLFWInit();

    const float XInitialRect = Definition.XPositionOnScreen;
    const float YInitialRect = Definition.YPositionOnScreen;

    const float WidthInitial = Definition.WidthDesiredOnScreen;
    const float HeightInitial = Definition.HeightDesiredOnScreen;

    int32 X = XInitialRect;
    int32 Y = YInitialRect;
    int32 Width = WidthInitial;
    int32 Height = HeightInitial;

    check(Width > 0 && Height > 0);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

#if RAPHAEL_SELECTED_RHI_VULKAN == 1
    check(glfwVulkanSupported());
#endif    // RAPHAEL_SELECTED_RHI_VULKAN

    glfwWindowHint(GLFW_RESIZABLE, InDefinition.HasSizingFrame);
    glfwWindowHint(GLFW_DECORATED, InDefinition.HasOsWindowBorder);
    p_Handle = glfwCreateWindow(Width, Height, Definition.Title.c_str(), nullptr, nullptr);
    if (!p_Handle) {
        LOG(LogWindow, Fatal, "Failed To create the GLFW Window");
        Utils::RequestExit(1);
    }
    SetupGLFWCallbacks();

    MoveWindow(X, Y);
}

void Window::ReshapeWindow(int32 X, int32 Y, int32 Width, int32 Height)
{
    glfwSetWindowPos(p_Handle, X, Y);
    glfwSetWindowSize(p_Handle, Width, Height);
}

void Window::MoveWindow(int32 X, int32 Y)
{
    glfwSetWindowPos(p_Handle, X, Y);
}

void Window::BringToFront(bool bForce)
{
    if (bForce) {
        glfwFocusWindow(p_Handle);
    } else {
        Show();
    }
}

void Window::Destroy()
{
    LOG(LogWindow, Info, "Destroying GLFW Window '{:p}'", (void*)p_Handle);
    glfwDestroyWindow(p_Handle);
    p_Handle = nullptr;
}

void Window::Minimize()
{
    glfwIconifyWindow(p_Handle);
}

void Window::Maximize()
{
    glfwMaximizeWindow(p_Handle);
}

void Window::Restore()
{
    glfwRestoreWindow(p_Handle);
}

void Window::Show()
{
    if (IsMinimized()) {
        Restore();
    }

    if (!bIsVisible) {
        bIsVisible = true;
        glfwShowWindow(p_Handle);
    }
}

void Window::Hide()
{
    if (bIsVisible) {
        bIsVisible = false;
        glfwHideWindow(p_Handle);
    }
}

bool Window::IsMaximized() const
{
    return glfwGetWindowAttrib(p_Handle, GLFW_MAXIMIZED);
}

bool Window::IsMinimized() const
{
    return glfwGetWindowAttrib(p_Handle, GLFW_ICONIFIED) || glfwGetWindowAttrib(p_Handle, GLFW_VISIBLE);
}

bool Window::IsVisible() const
{
    return bIsVisible;
}

void Window::AcceptInput(bool bEnable)
{
    (void)bEnable;
}

int32 Window::GetWindowBorderSize() const
{
    return 0;
}

int32 Window::GetWindowTitleBarSize() const
{
    return 0;
}

void Window::SetText(const std::string_view Text)
{
    glfwSetWindowTitle(p_Handle, Text.data());
}

void Window::DrawAttention()
{
    glfwRequestWindowAttention(p_Handle);
}

GLFWwindow* Window::GetHandle() const
{
    return p_Handle;
}

void Window::ProcessEvents()
{
    glfwPollEvents();
}

bool Window::InitializeGLFW()
{
    if (GGLFWInitialized) {
        return true;
    }
    LOG(LogWindow, Info, "Initializing GLFW.");

    if (glfwInit() == GLFW_FALSE) {
        const char* ErrorMessage = nullptr;
        glfwGetError(&ErrorMessage);
        if (ErrorMessage) {
            LOG(LogWindow, Warning, "Could not initialize GLFW: {}", ErrorMessage);
        }
        return false;
    }

    glfwSetErrorCallback([](int ErrorCode, const char* ErrorMessage) {
        LOG(LogWindow, Error, "GLFW Error ({}) \"{:s}\"", ErrorCode, ErrorMessage);
    });

    int Major = 0;
    int Minor = 0;
    int Revision = 0;
    glfwGetVersion(&Major, &Minor, &Revision);

    LOG(LogWindow, Info, "Initialized GLFW {:d}.{:d}.{:d}  (compiled against {:d}.{:d}.{:d})", Major, Minor, Revision,
        GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION);

    GGLFWInitialized = true;
    return true;
}

void Window::SetupGLFWCallbacks()
{
    glfwSetWindowUserPointer(p_Handle, &Definition.EventCallback);

    bool isRawMouseMotionSupported = glfwRawMouseMotionSupported();
    if (isRawMouseMotionSupported)
        glfwSetInputMode(p_Handle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    else
        LOG(LogWindow, Warning, "Raw mouse motion not supported.");

    // Set GLFW callbacks
    glfwSetWindowSizeCallback(p_Handle, [](GLFWwindow* window, int width, int height) {
        auto& Callback = *(static_cast<WindowDefinition::EventHandler*>(glfwGetWindowUserPointer(window)));
        WindowResizeEvent event((uint32_t)width, (uint32_t)height);
        Callback(event);
    });

    glfwSetWindowCloseCallback(p_Handle, [](GLFWwindow* window) {
        auto& Callback = *(static_cast<WindowDefinition::EventHandler*>(glfwGetWindowUserPointer(window)));
        WindowCloseEvent event;
        Callback(event);
    });

    glfwSetKeyCallback(p_Handle, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        (void)scancode;
        (void)mods;
        auto& Callback = *(static_cast<WindowDefinition::EventHandler*>(glfwGetWindowUserPointer(window)));
        switch (action) {
            case GLFW_PRESS: {
                KeyPressedEvent event((KeyCode)key, 0);
                Callback(event);
                break;
            }
            case GLFW_RELEASE: {
                KeyReleasedEvent event((KeyCode)key);
                Callback(event);
                break;
            }
            case GLFW_REPEAT: {
                KeyPressedEvent event((KeyCode)key, 1);
                Callback(event);
                break;
            }
        }
    });

    glfwSetCharCallback(p_Handle, [](GLFWwindow* window, uint32_t codepoint) {
        auto& Callback = *(static_cast<WindowDefinition::EventHandler*>(glfwGetWindowUserPointer(window)));
        KeyTypedEvent event((KeyCode)codepoint);
        Callback(event);
    });

    glfwSetMouseButtonCallback(p_Handle, [](GLFWwindow* window, int button, int action, int mods) {
        (void)mods;
        auto& Callback = *(static_cast<WindowDefinition::EventHandler*>(glfwGetWindowUserPointer(window)));
        switch (action) {
            case GLFW_PRESS: {
                MouseButtonPressedEvent event((MouseButton)button);
                Callback(event);
                break;
            }
            case GLFW_RELEASE: {
                MouseButtonReleasedEvent event((MouseButton)button);
                Callback(event);
                break;
            }
        }
    });

    glfwSetScrollCallback(p_Handle, [](GLFWwindow* window, double xOffset, double yOffset) {
        auto& Callback = *(static_cast<WindowDefinition::EventHandler*>(glfwGetWindowUserPointer(window)));
        MouseScrolledEvent event((float)xOffset, (float)yOffset);
        Callback(event);
    });

    glfwSetCursorPosCallback(p_Handle, [](GLFWwindow* window, double x, double y) {
        auto& Callback = *(static_cast<WindowDefinition::EventHandler*>(glfwGetWindowUserPointer(window)));
        MouseMovedEvent event((float)x, (float)y);
        Callback(event);
    });

    glfwSetWindowIconifyCallback(p_Handle, [](GLFWwindow* window, int iconified) {
        auto& Callback = *(static_cast<WindowDefinition::EventHandler*>(glfwGetWindowUserPointer(window)));
        WindowMinimizeEvent event((bool)iconified);
        Callback(event);
    });
}
