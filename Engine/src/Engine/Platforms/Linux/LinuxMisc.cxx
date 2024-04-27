#include "Engine/Platforms/Linux/LinuxMisc.hxx"

#include "Engine/Core/Window.hxx"

#include "Engine/Core/Memory/MiMalloc.hxx"
#include "Engine/Misc/Assertions.hxx"

#include <ModernDialogs.h>
#include <dlfcn.h>
#include <filesystem>
#include <xdg.hpp>

EBoxReturnType LinuxMisc::DisplayMessageBox(EBoxMessageType MsgType, const std::string Title, const std::string Text)
{
    if (!Window::EnsureGLFWInit()) {
        return GenericMisc::DisplayMessageBox(MsgType, Title, Text);
    }

    MD::Style Style = MD::Style::Error;
    MD::Buttons Button = MD::Buttons::OK;

    switch (MsgType) {
        case EBoxMessageType::Ok:
            Button = MD::Buttons::OK;
            break;
        case EBoxMessageType::YesNo:
            Button = MD::Buttons::YesNo;
            break;
    }

    MD::Selection Result = MD::ShowMsgBox(Title, Text, Style, Button);

    switch (Result) {
        case MD::Selection::Error:
            return GenericMisc::DisplayMessageBox(MsgType, Title, Text);
        case MD::Selection::OK:
            return EBoxReturnType::Ok;
        case MD::Selection::Cancel:
            return EBoxReturnType::Cancel;
        case MD::Selection::Yes:
            return EBoxReturnType::Yes;
        case MD::Selection::No:
            return EBoxReturnType::No;
        default:
            break;
    }
    return EBoxReturnType::Ok;
}

// ------------------ Linux External Module --------------------------

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
    checkNoReentry();
    void* const Ptr = std::malloc(sizeof(MiMalloc));
    new (Ptr) MiMalloc;
    return reinterpret_cast<MiMalloc*>(Ptr);
}

Ref<IExternalModule> LinuxMisc::LoadExternalModule(const std::string& ModuleName)
{
    return Ref<LinuxExternalModule>::CreateNamed(ModuleName, ModuleName);
}

std::filesystem::path LinuxMisc::GetConfigPath()
{
#ifndef NDEBUG
    return std::filesystem::current_path();
#else
    return xdg::ConfigHomeDir() / "RaphaelEngine";
#endif
}
