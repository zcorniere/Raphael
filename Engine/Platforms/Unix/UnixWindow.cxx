#include "Engine/Platforms/Unix/UnixWindow.hxx"

#include "Engine/Core/Events/KeyEvent.hxx"
#include "Engine/Core/Events/MouseEvent.hxx"
#include "Engine/Core/Events/WindowEvent.hxx"
#include "Engine/Core/Window/Input.hxx"
#include "Engine/Renderer/RendererAPI.hxx"

#include <GLFW/glfw3.h>

DECLARE_LOGGER_CATEGORY(Core, LogUnixWindow, Info)

namespace Raphael
{

static bool s_GLFWInitialized = false;

static void GLFWErrorCallback(int errorCode, const char *message)
{
    LOG(LogUnixWindow, Error, "GLFW Error({}) -> {} ", errorCode, message);
}

Window *Window::Create(const WindowSpecification &specification)
{
    return new UnixWindow(specification);
}

UnixWindow::UnixWindow(const WindowSpecification &specification): m_Specification(specification)
{
}

UnixWindow::~UnixWindow()
{
    Shutdown();
}

void UnixWindow::Init()
{
    m_Data.Title = m_Specification.Title;
    m_Data.Width = m_Specification.Width;
    m_Data.Height = m_Specification.Height;

    LOG(LogUnixWindow, Info, "Creating window {} ({}, {})", m_Specification.Title, m_Specification.Width,
        m_Specification.Height);

    if (!s_GLFWInitialized) {
        s_GLFWInitialized = glfwInit();
        checkMsg(s_GLFWInitialized, "Could not initialize GLFW!");
        glfwSetErrorCallback(GLFWErrorCallback);
    }

    if (RendererAPI::Current() == RendererAPIType::Vulkan) { glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); }

    glfwWindowHint(GLFW_DECORATED, m_Specification.Decorated);

    if (m_Specification.Fullscreen) {
        GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(primaryMonitor);

        glfwWindowHint(GLFW_DECORATED, false);
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

        m_Window = glfwCreateWindow(mode->width, mode->height, m_Data.Title.c_str(), primaryMonitor, nullptr);
    } else {
        m_Window = glfwCreateWindow((int)m_Specification.Width, (int)m_Specification.Height, m_Data.Title.c_str(),
                                    nullptr, nullptr);
    }

    m_RendererContext = RendererContext::Create();
    m_RendererContext->Init();

    glfwSetWindowUserPointer(m_Window, &m_Data);

    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(m_Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    } else {
        LOG(LogUnixWindow, Warn, "Raw mouse motion not supported.");
    }

    // Set GLFW callbacks
    {
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow *window, int width, int height) {
            WindowData &data = *((WindowData *)glfwGetWindowUserPointer(window));

            WindowResizeEvent event((uint32_t)width, (uint32_t)height);
            data.EventCallback(event);
            data.Width = width;
            data.Height = height;
        });

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow *window) {
            WindowData &data = *((WindowData *)glfwGetWindowUserPointer(window));

            WindowCloseEvent event;
            data.EventCallback(event);
        });

        glfwSetKeyCallback(m_Window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
            (void)mods;
            (void)scancode;
            WindowData &data = *((WindowData *)glfwGetWindowUserPointer(window));

            switch (action) {
                case GLFW_PRESS: {
                    KeyPressedEvent event((KeyCode)key, 0);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE: {
                    KeyReleasedEvent event((KeyCode)key);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_REPEAT: {
                    KeyPressedEvent event((KeyCode)key, 1);
                    data.EventCallback(event);
                    break;
                }
            }
        });

        glfwSetCharCallback(m_Window, [](GLFWwindow *window, uint32_t codepoint) {
            WindowData &data = *((WindowData *)glfwGetWindowUserPointer(window));

            KeyTypedEvent event((KeyCode)codepoint);
            data.EventCallback(event);
        });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow *window, int button, int action, int mods) {
            (void)mods;
            WindowData &data = *((WindowData *)glfwGetWindowUserPointer(window));

            switch (action) {
                case GLFW_PRESS: {
                    MouseButtonPressedEvent event((MouseButton)button);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE: {
                    MouseButtonReleasedEvent event((MouseButton)button);
                    data.EventCallback(event);
                    break;
                }
            }
        });

        glfwSetScrollCallback(m_Window, [](GLFWwindow *window, double xOffset, double yOffset) {
            WindowData &data = *((WindowData *)glfwGetWindowUserPointer(window));

            MouseScrolledEvent event((float)xOffset, (float)yOffset);
            data.EventCallback(event);
        });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow *window, double x, double y) {
            WindowData &data = *((WindowData *)glfwGetWindowUserPointer(window));
            MouseMovedEvent event((float)x, (float)y);
            data.EventCallback(event);
        });

        glfwSetWindowIconifyCallback(m_Window, [](GLFWwindow *window, int iconified) {
            WindowData &data = *((WindowData *)glfwGetWindowUserPointer(window));
            WindowMinimizeEvent event((bool)iconified);
            data.EventCallback(event);
        });
    }

    // Update window size to actual size
    {
        int width = 0;
        int height = 0;
        glfwGetWindowSize(m_Window, &width, &height);
        m_Data.Width = width;
        m_Data.Height = height;
    }
}

void UnixWindow::Shutdown()
{
    glfwTerminate();
    s_GLFWInitialized = false;
}

inline std::pair<float, float> UnixWindow::GetWindowPos() const
{
    int x, y;
    glfwGetWindowPos(m_Window, &x, &y);
    return {(float)x, (float)y};
}

void UnixWindow::ProcessEvents()
{
    glfwPollEvents();
    Input::Update();
}

void UnixWindow::SwapBuffers()
{
    // present
}

void UnixWindow::SetVSync(bool enabled)
{
    m_Specification.VSync = enabled;
}

bool UnixWindow::IsVSync() const
{
    return m_Specification.VSync;
}

void UnixWindow::SetResizable(bool resizable) const
{
    glfwSetWindowAttrib(m_Window, GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
}

void UnixWindow::Maximize()
{
    glfwMaximizeWindow(m_Window);
}

void UnixWindow::CenterWindow()
{
    const GLFWvidmode *videmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int x = (videmode->width / 2) - (m_Data.Width / 2);
    int y = (videmode->height / 2) - (m_Data.Height / 2);
    glfwSetWindowPos(m_Window, x, y);
}

void UnixWindow::SetTitle(const std::string &title)
{
    m_Data.Title = title;
    glfwSetWindowTitle(m_Window, m_Data.Title.c_str());
}

}    // namespace Raphael
