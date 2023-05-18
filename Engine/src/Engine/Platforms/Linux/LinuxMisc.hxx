#pragma once

#include "Engine/Platforms/PlatformMisc.hxx"

class LinuxExternalModule : public IExternalModule
{
public:
    LinuxExternalModule(std::string_view ModulePath);
    virtual ~LinuxExternalModule();

    virtual void *GetSymbol_Internal(std::string_view SymbolName) const override;

private:
    void *ModuleHandle;
};

class LinuxMisc : public GenericMisc
{
public:
    static EBoxReturnType DisplayMessageBox(EBoxMessageType MsgType, const std::string_view Text,
                                     const std::string_view Caption);

    static Ref<IExternalModule> LoadExternalModule(const std::string &ModuleName);

    static std::filesystem::path GetConfigPath();
};

using PlatformMisc = LinuxMisc;
