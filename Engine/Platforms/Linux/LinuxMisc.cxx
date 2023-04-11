#include "Engine/Platforms/Linux/LinuxMisc.hxx"

#include "Engine/Platforms/Linux/Window.hxx"

EBoxReturnType LinuxMisc::DisplayMessageBox(EBoxMessageType MsgType, const std::string_view Text,
                                     const std::string_view Caption)
{
    if (!Window::EnsureSDLInit()) { return GenericMisc::DisplayMessageBox(MsgType, Text, Caption); }

    std::vector<SDL_MessageBoxButtonData> Buttons;

    switch (MsgType) {
        case EBoxMessageType::Ok:
            Buttons.resize(1);
            Buttons[0] = {
                .flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT,
                .buttonid = (int)EBoxReturnType::Ok,
                .text = "0k",
            };
            break;

        case EBoxMessageType::YesNo:
            Buttons.resize(2);
            Buttons[0] = {
                .flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,
                .buttonid = (int)EBoxReturnType::Yes,
                .text = "Yes",
            };
            Buttons[1] = {
                .flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,
                .buttonid = (int)EBoxReturnType::No,
                .text = "No",
            };
            break;
    }

    SDL_MessageBoxData MessageBoxData{
        .flags = SDL_MESSAGEBOX_INFORMATION,
        .window = nullptr,
        .title = Caption.data(),
        .message = Text.data(),

        .numbuttons = static_cast<int>(Buttons.size()),
        .buttons = Buttons.data(),

        .colorScheme = nullptr,
    };
    int ButtonPressed = -1;
    EBoxReturnType Answer = EBoxReturnType::Cancel;

    if (SDL_ShowMessageBox(&MessageBoxData, &ButtonPressed) == -1) {
        return GenericMisc::DisplayMessageBox(MsgType, Text, Caption);
    } else {
        Answer = ButtonPressed == -1 ? EBoxReturnType::Cancel : static_cast<EBoxReturnType>(ButtonPressed);
    }
    return Answer;
}
