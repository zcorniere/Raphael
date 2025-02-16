#include "Engine/Core/Window.hxx"

#include "Engine/Core/Events/ApplicationEvent.hxx"
#include "Engine/Core/Events/KeyEvent.hxx"
#include "Engine/Core/Events/MouseEvent.hxx"
#include "Engine/Core/RHI/RHI.hxx"
#include "Engine/Misc/CommandLine.hxx"
#include "Engine/Misc/MiscDefines.hxx"
#include "Engine/Misc/Utils.hxx"

#include <GLFW/glfw3.h>

DECLARE_LOGGER_CATEGORY(Core, LogWindow, Info);

std::atomic_bool RWindow::bGLFWInitialized = false;
std::atomic_short RWindow::GFLWInUseCount = 0;

bool RWindow::EnsureGLFWInit()
{
    if (!InitializeGLFW()) {
        LOG(LogWindow, Fatal, "Window::Initialize() : Failed to initialize window");
        checkNoEntry();
        return false;
    }
    return true;
}

RWindow::RWindow(): Definition(), p_Handle(nullptr), bIsVisible(false)
{
}

RWindow::~RWindow()
{
    if (p_Handle) {
        Destroy();
    }
}

void RWindow::Initialize(const FWindowDefinition& InDefinition)
{
    GFLWInUseCount += 1;

    Definition = InDefinition;
    checkMsg(Definition.EventCallback, "You must provide an event callback !");

    EnsureGLFWInit();

    const int32 X = Definition.XPositionOnScreen;
    const int32 Y = Definition.YPositionOnScreen;
    const int32 Width = Definition.WidthDesiredOnScreen;
    const int32 Height = Definition.HeightDesiredOnScreen;

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

void RWindow::ReshapeWindow(int32 X, int32 Y, int32 Width, int32 Height)
{
    glfwSetWindowPos(p_Handle, X, Y);
    glfwSetWindowSize(p_Handle, Width, Height);
}

void RWindow::MoveWindow(int32 X, int32 Y)
{
    glfwSetWindowPos(p_Handle, X, Y);
}

void RWindow::BringToFront(bool bForce)
{
    if (bForce) {
        glfwFocusWindow(p_Handle);
    } else {
        Show();
    }
}

void RWindow::Destroy()
{
    RHI::FlushDeletionQueue();

    LOG(LogWindow, Info, "Destroying GLFW Window '{:p}'", (void*)p_Handle);
    glfwDestroyWindow(p_Handle);
    p_Handle = nullptr;

    GFLWInUseCount -= 1;
    if (GFLWInUseCount == 0) {
        LOG(LogWindow, Info, "Terminating GLFW.");
        glfwTerminate();
        bGLFWInitialized = false;
    }
}

void RWindow::Minimize()
{
    glfwIconifyWindow(p_Handle);
}

void RWindow::Maximize()
{
    glfwMaximizeWindow(p_Handle);
}

void RWindow::Restore()
{
    glfwRestoreWindow(p_Handle);
}

void RWindow::Show()
{
    if (IsMinimized()) {
        Restore();
    }

    if (!bIsVisible) {
        bIsVisible = true;
        glfwShowWindow(p_Handle);
    }
}

void RWindow::Hide()
{
    if (bIsVisible) {
        bIsVisible = false;
        glfwHideWindow(p_Handle);
    }
}

bool RWindow::IsMaximized() const
{
    return glfwGetWindowAttrib(p_Handle, GLFW_MAXIMIZED);
}

bool RWindow::IsMinimized() const
{
    return glfwGetWindowAttrib(p_Handle, GLFW_ICONIFIED) || glfwGetWindowAttrib(p_Handle, GLFW_VISIBLE);
}

bool RWindow::IsVisible() const
{
    return bIsVisible;
}

void RWindow::AcceptInput(bool bEnable)
{
    (void)bEnable;
}

int32 RWindow::GetWindowBorderSize() const
{
    return 0;
}

int32 RWindow::GetWindowTitleBarSize() const
{
    return 0;
}

void RWindow::SetText(const std::string_view Text)
{
    glfwSetWindowTitle(p_Handle, Text.data());
}

void RWindow::DrawAttention()
{
    glfwRequestWindowAttention(p_Handle);
}

GLFWwindow* RWindow::GetHandle() const
{
    return p_Handle;
}

void RWindow::ProcessEvents()
{
    glfwPollEvents();
}

namespace GLFWMemAllocator
{
void* Allocate(size_t Size, void*)
{
    return Memory::Malloc(Size, alignof(std::max_align_t));
}

void* Reallocate(void* Pointer, size_t Size, void*)
{
    return Memory::Realloc(Pointer, Size, alignof(std::max_align_t));
}

void Deallocate(void* Pointer, void*)
{
    Memory::Free(Pointer);
}

}    // namespace GLFWMemAllocator

bool RWindow::InitializeGLFW()
{
    if (bGLFWInitialized) {
        return true;
    }

    glfwSetErrorCallback([](int ErrorCode, const char* ErrorMessage) {
        LOG(LogWindow, Error, "GLFW Error ({}) \"{:s}\"", ErrorCode, ErrorMessage);
    });

    GLFWallocator allocator = {
        .allocate = GLFWMemAllocator::Allocate,
        .reallocate = GLFWMemAllocator::Reallocate,
        .deallocate = GLFWMemAllocator::Deallocate,
        .user = nullptr,
    };
    glfwInitAllocator(&allocator);

#if defined(PLATFORM_LINUX)
    if (glfwPlatformSupported(GLFW_PLATFORM_WAYLAND) && !FCommandLine::Param("-forceX11")) {
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
        LOG(LogWindow, Info, "Initializing GLFW - using Wayland");
    } else {
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
        LOG(LogWindow, Info, "Initializing GLFW - using X11");
    }
#elif define(PLATFORM_WINDOWS)
    LOG(LogWindow, Info, "Initializing GLFW.");
#endif

    if (glfwInit() == GLFW_FALSE) {
        const char* ErrorMessage = nullptr;
        glfwGetError(&ErrorMessage);
        if (ErrorMessage) {
            LOG(LogWindow, Warning, "Could not initialize GLFW: {}", ErrorMessage);
        }
        return false;
    }

    int Major = 0;
    int Minor = 0;
    int Revision = 0;
    glfwGetVersion(&Major, &Minor, &Revision);

    LOG(LogWindow, Info, "Initialized GLFW {:d}.{:d}.{:d}  (compiled against {:d}.{:d}.{:d})", Major, Minor, Revision,
        GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION);

    bGLFWInitialized = true;
    return true;
}

void RWindow::SetupGLFWCallbacks()
{
    glfwSetWindowUserPointer(p_Handle, &Definition.EventCallback);

    bool isRawMouseMotionSupported = glfwRawMouseMotionSupported();
    if (isRawMouseMotionSupported)
        glfwSetInputMode(p_Handle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    else
        LOG(LogWindow, Warning, "Raw mouse motion not supported.");

    // Set GLFW callbacks
    glfwSetWindowSizeCallback(p_Handle, [](GLFWwindow* window, int width, int height) {
        auto& Callback = *(static_cast<FWindowDefinition::EventHandler*>(glfwGetWindowUserPointer(window)));
        FWindowResizeEvent event((uint32_t)width, (uint32_t)height);
        Callback(event);
    });

    glfwSetWindowCloseCallback(p_Handle, [](GLFWwindow* window) {
        auto& Callback = *(static_cast<FWindowDefinition::EventHandler*>(glfwGetWindowUserPointer(window)));
        FWindowCloseEvent event;
        Callback(event);
    });

    glfwSetKeyCallback(p_Handle, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        (void)scancode;
        (void)mods;
        auto& Callback = *(static_cast<FWindowDefinition::EventHandler*>(glfwGetWindowUserPointer(window)));
        switch (action) {
            case GLFW_PRESS: {
                FKeyPressedEvent event((EKeyCode)key, 0);
                Callback(event);
                break;
            }
            case GLFW_RELEASE: {
                FKeyReleasedEvent event((EKeyCode)key);
                Callback(event);
                break;
            }
            case GLFW_REPEAT: {
                FKeyPressedEvent event((EKeyCode)key, 1);
                Callback(event);
                break;
            }
        }
    });

    glfwSetCharCallback(p_Handle, [](GLFWwindow* window, uint32_t codepoint) {
        auto& Callback = *(static_cast<FWindowDefinition::EventHandler*>(glfwGetWindowUserPointer(window)));
        FKeyTypedEvent event((EKeyCode)codepoint);
        Callback(event);
    });

    glfwSetMouseButtonCallback(p_Handle, [](GLFWwindow* window, int button, int action, int mods) {
        (void)mods;
        auto& Callback = *(static_cast<FWindowDefinition::EventHandler*>(glfwGetWindowUserPointer(window)));
        switch (action) {
            case GLFW_PRESS: {
                FMouseButtonPressedEvent event((EMouseButton)button);
                Callback(event);
                break;
            }
            case GLFW_RELEASE: {
                FMouseButtonReleasedEvent event((EMouseButton)button);
                Callback(event);
                break;
            }
        }
    });

    glfwSetScrollCallback(p_Handle, [](GLFWwindow* window, double xOffset, double yOffset) {
        auto& Callback = *(static_cast<FWindowDefinition::EventHandler*>(glfwGetWindowUserPointer(window)));
        FMouseScrolledEvent event((float)xOffset, (float)yOffset);
        Callback(event);
    });

    glfwSetCursorPosCallback(p_Handle, [](GLFWwindow* window, double x, double y) {
        auto& Callback = *(static_cast<FWindowDefinition::EventHandler*>(glfwGetWindowUserPointer(window)));
        FMouseMovedEvent event((float)x, (float)y);
        Callback(event);
    });

    glfwSetWindowIconifyCallback(p_Handle, [](GLFWwindow* window, int iconified) {
        auto& Callback = *(static_cast<FWindowDefinition::EventHandler*>(glfwGetWindowUserPointer(window)));
        FWindowMinimizeEvent event((bool)iconified);
        Callback(event);
    });
}
