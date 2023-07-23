#include "Engine/Platforms/Linux/LinuxMisc.hxx"

#include "Engine/Core/Window.hxx"

#include "Engine/Core/Memory/MiMalloc.hxx"
#include "Engine/Misc/Utils.hxx"

#include <dlfcn.h>
#include <xdg.hpp>

EBoxReturnType LinuxMisc::DisplayMessageBox(EBoxMessageType MsgType, const std::string_view Text,
                                            const std::string_view Caption)
{
    if (!Window::EnsureGLFWInit()) {
        return GenericMisc::DisplayMessageBox(MsgType, Text, Caption);
    }
    return GenericMisc::DisplayMessageBox(MsgType, Text, Caption);

    // std::vector<SDL_MessageBoxButtonData> Buttons;

    // switch (MsgType) {
    //     case EBoxMessageType::Ok:
    //         Buttons.resize(1);
    //         Buttons[0] = {
    //             .flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT,
    //             .buttonid = (int)EBoxReturnType::Ok,
    //             .text = "0k",
    //         };
    //         break;

    //     case EBoxMessageType::YesNo:
    //         Buttons.resize(2);
    //         Buttons[0] = {
    //             .flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,
    //             .buttonid = (int)EBoxReturnType::Yes,
    //             .text = "Yes",
    //         };
    //         Buttons[1] = {
    //             .flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,
    //             .buttonid = (int)EBoxReturnType::No,
    //             .text = "No",
    //         };
    //         break;
    // }

    // SDL_MessageBoxData MessageBoxData{
    //     .flags = SDL_MESSAGEBOX_INFORMATION,
    //     .window = nullptr,
    //     .title = Caption.data(),
    //     .message = Text.data(),

    //     .numbuttons = static_cast<int>(Buttons.size()),
    //     .buttons = Buttons.data(),

    //     .colorScheme = nullptr,
    // };
    // int ButtonPressed = -1;
    // EBoxReturnType Answer = EBoxReturnType::Cancel;

    // if (SDL_ShowMessageBox(&MessageBoxData, &ButtonPressed) == -1) {
    //     return GenericMisc::DisplayMessageBox(MsgType, Text, Caption);
    // } else {
    //     Answer = ButtonPressed == -1 ? EBoxReturnType::Cancel : static_cast<EBoxReturnType>(ButtonPressed);
    // }
    // return Answer;
}

// ------------------ Linux External Module --------------------------

static std::unordered_map<std::string, WeakRef<LinuxExternalModule>> s_ModuleStorage;

LinuxExternalModule::LinuxExternalModule(std::string_view ModulePath): IExternalModule(ModulePath)
{
    ModuleHandle = dlopen(ModulePath.data(), RTLD_NOW | RTLD_LOCAL);
}

LinuxExternalModule::~LinuxExternalModule()
{
    dlclose(ModuleHandle);
}

void* LinuxExternalModule::GetSymbol_Internal(std::string_view SymbolName) const
{
    return dlsym(ModuleHandle, SymbolName.data());
}

Malloc* LinuxMisc::BaseAllocator()
{
    // This function gets executed very early, way before main() (because global constructors will allocate memory).
    // This makes it an ideal place for a root privilege check.
    if (geteuid() == 0) {
        fprintf(stderr, "Refusing to run with the root privileges.\n");
        Utils::RequestExit(true);
        return nullptr;
    }

    void* Ptr = std::malloc(sizeof(MiMalloc));
    new (Ptr) MiMalloc;
    return reinterpret_cast<MiMalloc*>(Ptr);
}

Ref<IExternalModule> LinuxMisc::LoadExternalModule(const std::string& ModuleName)
{
    auto Iter = s_ModuleStorage.find(ModuleName);

    if (Iter == s_ModuleStorage.end() || !Iter->second.IsValid()) {
        Ref<LinuxExternalModule> Module = Ref<LinuxExternalModule>::CreateNamed(ModuleName, ModuleName);
        s_ModuleStorage[ModuleName] = Module;
        return Module;
    }

    return Ref(Iter->second);
}

std::filesystem::path LinuxMisc::GetConfigPath()
{
    return xdg::ConfigHomeDir() / "RaphaelEngine";
}
