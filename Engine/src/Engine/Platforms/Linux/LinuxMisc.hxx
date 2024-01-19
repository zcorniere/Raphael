#pragma once

#include "Engine/Platforms/PlatformMisc.hxx"

/// @brief Linux implementation of the IExternalModule interface
class LinuxExternalModule : public IExternalModule
{
public:
    /// @copydoc IExternalModule::IExternalModule
    explicit LinuxExternalModule(std::string_view ModulePath);
    virtual ~LinuxExternalModule();

private:
    virtual void* GetSymbol_Internal(std::string_view SymbolName) const override;

private:
    void* ModuleHandle;
};

/// @brief Miscellaneous Linux feature
class LinuxMisc : public GenericMisc
{
public:
    /// @copydoc GenericMisc::DisplayMessageBox
    static EBoxReturnType DisplayMessageBox(EBoxMessageType MsgType, const std::string Title, const std::string Text);

    static Malloc* BaseAllocator();

    /// @copydoc GenericMisc::LoadExternalModule
    static Ref<IExternalModule> LoadExternalModule(const std::string& ModuleName);

    /// @brief Return the XDG_CONFIG path
    static std::filesystem::path GetConfigPath();
};

// Helper to use the platform implementation
using PlatformMisc = LinuxMisc;
