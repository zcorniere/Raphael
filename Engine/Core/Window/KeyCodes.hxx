#pragma once

namespace Raphael
{

enum class KeyCode : uint16_t {
    // From glfw3.h
    Space = 32,
    Apostrophe = 39, /* ' */
    Comma = 44,      /* , */
    Minus = 45,      /* - */
    Period = 46,     /* . */
    Slash = 47,      /* / */

    D0 = 48, /* 0 */
    D1 = 49, /* 1 */
    D2 = 50, /* 2 */
    D3 = 51, /* 3 */
    D4 = 52, /* 4 */
    D5 = 53, /* 5 */
    D6 = 54, /* 6 */
    D7 = 55, /* 7 */
    D8 = 56, /* 8 */
    D9 = 57, /* 9 */

    Semicolon = 59, /* ; */
    Equal = 61,     /* = */

    A = 65,
    B = 66,
    C = 67,
    D = 68,
    E = 69,
    F = 70,
    G = 71,
    H = 72,
    I = 73,
    J = 74,
    K = 75,
    L = 76,
    M = 77,
    N = 78,
    O = 79,
    P = 80,
    Q = 81,
    R = 82,
    S = 83,
    T = 84,
    U = 85,
    V = 86,
    W = 87,
    X = 88,
    Y = 89,
    Z = 90,

    LeftBracket = 91,  /* [ */
    Backslash = 92,    /* \ */
    RightBracket = 93, /* ] */
    GraveAccent = 96,  /* ` */

    World1 = 161, /* non-US #1 */
    World2 = 162, /* non-US #2 */

    /* Function keys */
    Escape = 256,
    Enter = 257,
    Tab = 258,
    Backspace = 259,
    Insert = 260,
    Delete = 261,
    Right = 262,
    Left = 263,
    Down = 264,
    Up = 265,
    PageUp = 266,
    PageDown = 267,
    Home = 268,
    End = 269,
    CapsLock = 280,
    ScrollLock = 281,
    NumLock = 282,
    PrintScreen = 283,
    Pause = 284,
    F1 = 290,
    F2 = 291,
    F3 = 292,
    F4 = 293,
    F5 = 294,
    F6 = 295,
    F7 = 296,
    F8 = 297,
    F9 = 298,
    F10 = 299,
    F11 = 300,
    F12 = 301,
    F13 = 302,
    F14 = 303,
    F15 = 304,
    F16 = 305,
    F17 = 306,
    F18 = 307,
    F19 = 308,
    F20 = 309,
    F21 = 310,
    F22 = 311,
    F23 = 312,
    F24 = 313,
    F25 = 314,

    /* Keypad */
    KP0 = 320,
    KP1 = 321,
    KP2 = 322,
    KP3 = 323,
    KP4 = 324,
    KP5 = 325,
    KP6 = 326,
    KP7 = 327,
    KP8 = 328,
    KP9 = 329,
    KPDecimal = 330,
    KPDivide = 331,
    KPMultiply = 332,
    KPSubtract = 333,
    KPAdd = 334,
    KPEnter = 335,
    KPEqual = 336,

    LeftShift = 340,
    LeftControl = 341,
    LeftAlt = 342,
    LeftSuper = 343,
    RightShift = 344,
    RightControl = 345,
    RightAlt = 346,
    RightSuper = 347,
    Menu = 348
};

using Key = KeyCode;

enum class CursorMode {
    Normal = 0,
    Hidden = 1,
    Locked = 2,
};

typedef enum class MouseButton : uint16_t {
    Button0 = 0,
    Button1 = 1,
    Button2 = 2,
    Button3 = 3,
    Button4 = 4,
    Button5 = 5,
    Left = Button0,
    Right = Button1,
    Middle = Button2
} Button;

inline std::ostream &operator<<(std::ostream &os, KeyCode keyCode)
{
    os << static_cast<int32_t>(keyCode);
    return os;
}

inline std::ostream &operator<<(std::ostream &os, MouseButton button)
{
    os << static_cast<int32_t>(button);
    return os;
}
}    // namespace Raphael

// From glfw3.h
#define RPH_KEY_SPACE ::Raphael::Key::Space
#define RPH_KEY_APOSTROPHE ::Raphael::Key::Apostrophe /* ' */
#define RPH_KEY_COMMA ::Raphael::Key::Comma           /* , */
#define RPH_KEY_MINUS ::Raphael::Key::Minus           /* - */
#define RPH_KEY_PERIOD ::Raphael::Key::Period         /* . */
#define RPH_KEY_SLASH ::Raphael::Key::Slash           /* / */
#define RPH_KEY_0 ::Raphael::Key::D0
#define RPH_KEY_1 ::Raphael::Key::D1
#define RPH_KEY_2 ::Raphael::Key::D2
#define RPH_KEY_3 ::Raphael::Key::D3
#define RPH_KEY_4 ::Raphael::Key::D4
#define RPH_KEY_5 ::Raphael::Key::D5
#define RPH_KEY_6 ::Raphael::Key::D6
#define RPH_KEY_7 ::Raphael::Key::D7
#define RPH_KEY_8 ::Raphael::Key::D8
#define RPH_KEY_9 ::Raphael::Key::D9
#define RPH_KEY_SEMICOLON ::Raphael::Key::Semicolon /* ; */
#define RPH_KEY_EQUAL ::Raphael::Key::Equal         /* = */
#define RPH_KEY_A ::Raphael::Key::A
#define RPH_KEY_B ::Raphael::Key::B
#define RPH_KEY_C ::Raphael::Key::C
#define RPH_KEY_D ::Raphael::Key::D
#define RPH_KEY_E ::Raphael::Key::E
#define RPH_KEY_F ::Raphael::Key::F
#define RPH_KEY_G ::Raphael::Key::G
#define RPH_KEY_H ::Raphael::Key::H
#define RPH_KEY_I ::Raphael::Key::I
#define RPH_KEY_J ::Raphael::Key::J
#define RPH_KEY_K ::Raphael::Key::K
#define RPH_KEY_L ::Raphael::Key::L
#define RPH_KEY_M ::Raphael::Key::M
#define RPH_KEY_N ::Raphael::Key::N
#define RPH_KEY_O ::Raphael::Key::O
#define RPH_KEY_P ::Raphael::Key::P
#define RPH_KEY_Q ::Raphael::Key::Q
#define RPH_KEY_R ::Raphael::Key::R
#define RPH_KEY_S ::Raphael::Key::S
#define RPH_KEY_T ::Raphael::Key::T
#define RPH_KEY_U ::Raphael::Key::U
#define RPH_KEY_V ::Raphael::Key::V
#define RPH_KEY_W ::Raphael::Key::W
#define RPH_KEY_X ::Raphael::Key::X
#define RPH_KEY_Y ::Raphael::Key::Y
#define RPH_KEY_Z ::Raphael::Key::Z
#define RPH_KEY_LEFT_BRACKET ::Raphael::Key::LeftBracket   /* [ */
#define RPH_KEY_BACKSLASH ::Raphael::Key::Backslash        /* \ */
#define RPH_KEY_RIGHT_BRACKET ::Raphael::Key::RightBracket /* ] */
#define RPH_KEY_GRAVE_ACCENT ::Raphael::Key::GraveAccent   /* ` */
#define RPH_KEY_WORLD_1 ::Raphael::Key::World1             /* non-US #1 */
#define RPH_KEY_WORLD_2 ::Raphael::Key::World2             /* non-US #2 */

/* Function keys */
#define RPH_KEY_ESCAPE ::Raphael::Key::Escape
#define RPH_KEY_ENTER ::Raphael::Key::Enter
#define RPH_KEY_TAB ::Raphael::Key::Tab
#define RPH_KEY_BACKSPACE ::Raphael::Key::Backspace
#define RPH_KEY_INSERT ::Raphael::Key::Insert
#define RPH_KEY_DELETE ::Raphael::Key::Delete
#define RPH_KEY_RIGHT ::Raphael::Key::Right
#define RPH_KEY_LEFT ::Raphael::Key::Left
#define RPH_KEY_DOWN ::Raphael::Key::Down
#define RPH_KEY_UP ::Raphael::Key::Up
#define RPH_KEY_PAGE_UP ::Raphael::Key::PageUp
#define RPH_KEY_PAGE_DOWN ::Raphael::Key::PageDown
#define RPH_KEY_HOME ::Raphael::Key::Home
#define RPH_KEY_END ::Raphael::Key::End
#define RPH_KEY_CAPS_LOCK ::Raphael::Key::CapsLock
#define RPH_KEY_SCROLL_LOCK ::Raphael::Key::ScrollLock
#define RPH_KEY_NUM_LOCK ::Raphael::Key::NumLock
#define RPH_KEY_PRINT_SCREEN ::Raphael::Key::PrintScreen
#define RPH_KEY_PAUSE ::Raphael::Key::Pause
#define RPH_KEY_F1 ::Raphael::Key::F1
#define RPH_KEY_F2 ::Raphael::Key::F2
#define RPH_KEY_F3 ::Raphael::Key::F3
#define RPH_KEY_F4 ::Raphael::Key::F4
#define RPH_KEY_F5 ::Raphael::Key::F5
#define RPH_KEY_F6 ::Raphael::Key::F6
#define RPH_KEY_F7 ::Raphael::Key::F7
#define RPH_KEY_F8 ::Raphael::Key::F8
#define RPH_KEY_F9 ::Raphael::Key::F9
#define RPH_KEY_F10 ::Raphael::Key::F10
#define RPH_KEY_F11 ::Raphael::Key::F11
#define RPH_KEY_F12 ::Raphael::Key::F12
#define RPH_KEY_F13 ::Raphael::Key::F13
#define RPH_KEY_F14 ::Raphael::Key::F14
#define RPH_KEY_F15 ::Raphael::Key::F15
#define RPH_KEY_F16 ::Raphael::Key::F16
#define RPH_KEY_F17 ::Raphael::Key::F17
#define RPH_KEY_F18 ::Raphael::Key::F18
#define RPH_KEY_F19 ::Raphael::Key::F19
#define RPH_KEY_F20 ::Raphael::Key::F20
#define RPH_KEY_F21 ::Raphael::Key::F21
#define RPH_KEY_F22 ::Raphael::Key::F22
#define RPH_KEY_F23 ::Raphael::Key::F23
#define RPH_KEY_F24 ::Raphael::Key::F24
#define RPH_KEY_F25 ::Raphael::Key::F25

/* Keypad */
#define RPH_KEY_KP_0 ::Raphael::Key::KP0
#define RPH_KEY_KP_1 ::Raphael::Key::KP1
#define RPH_KEY_KP_2 ::Raphael::Key::KP2
#define RPH_KEY_KP_3 ::Raphael::Key::KP3
#define RPH_KEY_KP_4 ::Raphael::Key::KP4
#define RPH_KEY_KP_5 ::Raphael::Key::KP5
#define RPH_KEY_KP_6 ::Raphael::Key::KP6
#define RPH_KEY_KP_7 ::Raphael::Key::KP7
#define RPH_KEY_KP_8 ::Raphael::Key::KP8
#define RPH_KEY_KP_9 ::Raphael::Key::KP9
#define RPH_KEY_KP_DECIMAL ::Raphael::Key::KPDecimal
#define RPH_KEY_KP_DIVIDE ::Raphael::Key::KPDivide
#define RPH_KEY_KP_MULTIPLY ::Raphael::Key::KPMultiply
#define RPH_KEY_KP_SUBTRACT ::Raphael::Key::KPSubtract
#define RPH_KEY_KP_ADD ::Raphael::Key::KPAdd
#define RPH_KEY_KP_ENTER ::Raphael::Key::KPEnter
#define RPH_KEY_KP_EQUAL ::Raphael::Key::KPEqual

#define RPH_KEY_LEFT_SHIFT ::Raphael::Key::LeftShift
#define RPH_KEY_LEFT_CONTROL ::Raphael::Key::LeftControl
#define RPH_KEY_LEFT_ALT ::Raphael::Key::LeftAlt
#define RPH_KEY_LEFT_SUPER ::Raphael::Key::LeftSuper
#define RPH_KEY_RIGHT_SHIFT ::Raphael::Key::RightShift
#define RPH_KEY_RIGHT_CONTROL ::Raphael::Key::RightControl
#define RPH_KEY_RIGHT_ALT ::Raphael::Key::RightAlt
#define RPH_KEY_RIGHT_SUPER ::Raphael::Key::RightSuper
#define RPH_KEY_MENU ::Raphael::Key::Menu

// Mouse
#define RPH_MOUSE_BUTTON_LEFT ::Raphael::Button::Left
#define RPH_MOUSE_BUTTON_RIGHT ::Raphael::Button::Right
#define RPH_MOUSE_BUTTON_MIDDLE ::Raphael::Button::Middle
