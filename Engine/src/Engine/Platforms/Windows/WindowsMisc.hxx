#pragma once

#include "Engine/Platforms/PlatformMisc.hxx"

class WindowsExternalModule : public IExternalModule
{
public:
    WindowsExternalModule(std::string_view ModulePath);
    virtual ~WindowsExternalModule();

    virtual void *GetSymbol_Internal(std::string_view SymbolName) const override;

private:
    HMODULE ModuleHandle;
};

class WindowsMisc : public GenericMisc
{
public:
    static EBoxReturnType DisplayMessageBox(EBoxMessageType MsgType, const std::string_view Text,
                                     const std::string_view Caption);

    static Ref<IExternalModule> LoadExternalModule(const std::string &ModuleName);

    static std::filesystem::path GetConfigPath();
};

using PlatformMisc = WindowsMisc;
