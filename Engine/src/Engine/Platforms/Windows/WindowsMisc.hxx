#pragma once

#include "Engine/Platforms/PlatformMisc.hxx"
// IWYU pragma: private, include "FPlatformMisc.hxx"

class RWindowsExternalModule : public IExternalModule
{
    RTTI_DECLARE_TYPEINFO(RWindowsExternalModule, IExternalModule);

public:
    RWindowsExternalModule(std::string_view ModulePath);
    virtual ~RWindowsExternalModule();

    virtual void* GetSymbol_Internal(std::string_view SymbolName) const override;

private:
    void* ModuleHandle = nullptr;
};

class FWindowsMisc : public FGenericMisc
{
public:
    static EBoxReturnType DisplayMessageBox(EBoxMessageType MsgType, const std::string_view Text,
                                            const std::string_view Caption);

    static bool BaseAllocator(void* TargetMemory);

    static Ref<IExternalModule> LoadExternalModule(const std::string& ModuleName);

    static std::filesystem::path GetConfigPath();
};

using FPlatformMisc = FWindowsMisc;
