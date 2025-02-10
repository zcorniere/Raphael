#include "Engine/Platforms/Linux/LinuxMisc.hxx"

#include "Engine/Core/Window.hxx"

#include "Engine/Core/Memory/MiMalloc.hxx"
#include "Engine/Core/Memory/StdMalloc.hxx"
#include "Engine/Misc/Assertions.hxx"
#include "Engine/Misc/CommandLine.hxx"

#include <ModernDialogs.h>
#include <dlfcn.h>
#include <filesystem>
#include <xdg.hpp>

EBoxReturnType FLinuxMisc::DisplayMessageBox(EBoxMessageType MsgType, const std::string Title, const std::string Text)
{
    if (!RWindow::EnsureGLFWInit()) {
        return FGenericMisc::DisplayMessageBox(MsgType, Title, Text);
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
            return FGenericMisc::DisplayMessageBox(MsgType, Title, Text);
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

RLinuxExternalModule::RLinuxExternalModule(std::string_view ModulePath): IExternalModule(ModulePath)
{
    ModuleHandle = dlopen(ModulePath.data(), RTLD_NOW | RTLD_LOCAL);
}

RLinuxExternalModule::~RLinuxExternalModule()
{
    dlclose(ModuleHandle);
}

void* RLinuxExternalModule::GetSymbol_Internal(std::string_view SymbolName) const
{
    return dlsym(ModuleHandle, SymbolName.data());
}

bool FLinuxMisc::BaseAllocator(void* TargetMemory)
{
    checkNoReentry();

    if (FCommandLine::Param("usemimalloc")) {
        new (TargetMemory) FMiMalloc;
    } else {
        new (TargetMemory) FStdMalloc;
    }
    return true;
}

Ref<IExternalModule> FLinuxMisc::LoadExternalModule(const std::string& ModuleName)
{
    return Ref<RLinuxExternalModule>::CreateNamed(ModuleName, ModuleName);
}

std::filesystem::path FLinuxMisc::GetConfigPath()
{
#ifndef NDEBUG
    return std::filesystem::current_path();
#else
    return xdg::ConfigHomeDir() / "RaphaelEngine";
#endif
}
