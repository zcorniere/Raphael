#pragma once

#include "Engine/Platforms/PlatformMisc.hxx"
// IWYU pragma: private, include "FPlatformMisc.hxx"

/// @brief Linux implementation of the IExternalModule interface
class RLinuxExternalModule : public IExternalModule
{
    RTTI_DECLARE_TYPEINFO(RLinuxExternalModule, IExternalModule);

public:
    /// @copydoc IExternalModule::IExternalModule
    explicit RLinuxExternalModule(std::string_view ModulePath);
    virtual ~RLinuxExternalModule();

private:
    virtual void* GetSymbol_Internal(std::string_view SymbolName) const override;

private:
    void* ModuleHandle;
};

/// @brief Miscellaneous Linux feature
class FLinuxMisc : public FGenericMisc
{
public:
    /// @copydoc GenericMisc::DisplayMessageBox
    static EBoxReturnType DisplayMessageBox(EBoxMessageType MsgType, const std::string Title, const std::string Text);

    static IMallocInterface* BaseAllocator();

    /// @copydoc GenericMisc::LoadExternalModule
    static Ref<IExternalModule> LoadExternalModule(const std::string& ModuleName);

    /// @brief Return the XDG_CONFIG path
    static std::filesystem::path GetConfigPath();
};

// Helper to use the platform implementation
using FPlatformMisc = FLinuxMisc;
