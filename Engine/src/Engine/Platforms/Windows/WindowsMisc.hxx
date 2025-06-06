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
    /// @copydoc FGenericMisc::DisplayMessageBox
    static EBoxReturnType DisplayMessageBox(EBoxMessageType MsgType, const std::string_view Text,
                                            const std::string_view Caption);

    /// @copydoc FGenericMisc::GetCPUInformation
    static const FCPUInformation& GetCPUInformation();

    /// @copydoc FGenericMisc::BaseAllocator
    static bool BaseAllocator(void* TargetMemory);

    /// @copydoc FGenericMisc::LoadExternalModule
    static Ref<IExternalModule> LoadExternalModule(const std::string& ModuleName);

    /// @copydoc FGenericMisc::GetConfigPath
    static std::filesystem::path GetConfigPath();
};

using FPlatformMisc = FWindowsMisc;
