#include "Engine/Platforms/Windows/WindowsMisc.hxx"

#include "Engine/Core/Memory/MiMalloc.hxx"

#include <libloaderapi.h>
#include <shlobj_core.h>
#include <winuser.h>

EBoxReturnType WindowsMisc::DisplayMessageBox(EBoxMessageType MsgType, const std::string_view Text,
                                              const std::string_view Caption)
{
    unsigned WindowFlags = MB_ICONWARNING;
    switch (MsgType) {
        case EBoxMessageType::Ok:
            WindowFlags |= MB_OK;
            break;

        case EBoxMessageType::YesNo:
            WindowFlags |= MB_YESNO;
            break;
        default:
            checkNoEntry();
            break;
    }

    int Result = ::MessageBox(nullptr, Text.data(), Caption.data(), WindowFlags);

    switch (Result) {
        case IDYES:
            return EBoxReturnType::Yes;
        case IDNO:
            return EBoxReturnType::No;
        case IDOK:
            return EBoxReturnType::Ok;
        case IDCANCEL:
            return EBoxReturnType::Cancel;
    }
    return EBoxReturnType::Cancel;
}

// ------------------ Windows External Module --------------------------

static std::unordered_map<std::string, WeakRef<WindowsExternalModule>> s_ModuleStorage;

WindowsExternalModule::WindowsExternalModule(std::string_view ModulePath): IExternalModule(ModulePath)
{
    ModuleHandle = ::LoadLibrary(ModulePath.data());
}

WindowsExternalModule::~WindowsExternalModule()
{
    ::FreeLibrary(ModuleHandle);
}

void* WindowsExternalModule::GetSymbol_Internal(std::string_view SymbolName) const
{
    return ::GetProcAddress(ModuleHandle, SymbolName.data());
}

Malloc* WindowsMisc::BaseAllocator()
{
    void* Ptr = std::malloc(sizeof(MiMalloc));
    new (Ptr) MiMalloc;
    return reinterpret_cast<MiMalloc*>(Ptr);
}

Ref<IExternalModule> WindowsMisc::LoadExternalModule(const std::string& ModuleName)
{
    auto Iter = s_ModuleStorage.find(ModuleName);

    if (Iter == s_ModuleStorage.end() || !Iter->second.IsValid()) {
        Ref<WindowsExternalModule> Module = Ref<WindowsExternalModule>::Create(ModuleName);
        s_ModuleStorage[ModuleName] = Module;
        return Module;
    }
    return Ref(Iter->second);
}

std::filesystem::path WindowsMisc::GetConfigPath()
{
    std::filesystem::path returnPath = std::filesystem::current_path();
    PWSTR wAppDataPath = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &wAppDataPath))) {
        returnPath = std::filesystem::path(wAppDataPath);
        CoTaskMemFree(wAppDataPath);
    }

    return returnPath;
}
