#include "Engine/Platforms/Windows/WindowsMisc.hxx"

#include "Engine/Core/Memory/MiMalloc.hxx"
#include "Engine/Core/Memory/StdMalloc.hxx"
#include "Engine/Misc/Assertions.hxx"
#include "Engine/Misc/CommandLine.hxx"

#include <windows.h>

#include <libloaderapi.h>
#include <shlobj_core.h>

EBoxReturnType FWindowsMisc::DisplayMessageBox(EBoxMessageType MsgType, const std::string_view Text,
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

static TMap<std::string, WeakRef<RWindowsExternalModule>> s_ModuleStorage;

RWindowsExternalModule::RWindowsExternalModule(std::string_view ModulePath): IExternalModule(ModulePath)
{
    ModuleHandle = ::LoadLibrary(ModulePath.data());
}

RWindowsExternalModule::~RWindowsExternalModule()
{
    ::FreeLibrary(HMODULE(ModuleHandle));
}

void* RWindowsExternalModule::GetSymbol_Internal(std::string_view SymbolName) const
{
    return ::GetProcAddress(HMODULE(ModuleHandle), SymbolName.data());
}

bool FWindowsMisc::BaseAllocator(void* TargetMemory)
{
    checkNoReentry();

    if (FCommandLine::Param("usemimalloc")) {
        new (TargetMemory) FMiMalloc;
    } else {
        new (TargetMemory) FStdMalloc;
    }
    return true;
}

Ref<IExternalModule> FWindowsMisc::LoadExternalModule(const std::string& ModuleName)
{
    WeakRef<RWindowsExternalModule>* Iter = s_ModuleStorage.Find(ModuleName);

    if (Iter == nullptr || !Iter->IsValid()) {
        Ref<RWindowsExternalModule> Module = Ref<RWindowsExternalModule>::Create(ModuleName);
        s_ModuleStorage.FindOrAdd(ModuleName) = Module;
        return Module;
    }
    return Iter->Pin();
}

std::filesystem::path FWindowsMisc::GetConfigPath()
{
    std::filesystem::path returnPath = std::filesystem::current_path();
#ifdef NDEBUG
    PWSTR wAppDataPath = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &wAppDataPath))) {
        returnPath = std::filesystem::path(wAppDataPath);
        CoTaskMemFree(wAppDataPath);
    }
#endif

    return returnPath;
}
