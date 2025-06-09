#include "Engine/Platforms/Windows/WindowsMisc.hxx"

#include "Engine/Core/Memory/MiMalloc.hxx"
#include "Engine/Core/Memory/StdMalloc.hxx"
#include "Engine/Misc/Assertions.hxx"
#include "Engine/Misc/CommandLine.hxx"

#include <windows.h>

#include "WindowsMisc.hxx"
#include <libloaderapi.h>
#include <shlobj_core.h>

EBoxReturnType FWindowsMisc::DisplayMessageBox(EBoxMessageType MsgType, const std::string_view Text,
                                               const std::string_view Caption)
{
    unsigned WindowFlags = MB_ICONWARNING;
    switch (MsgType)
    {
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

    switch (Result)
    {
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

static void GetCPUVendor(char (&OutBuffer)[12 + 1])
{
    union
    {
        int8 Buffer[12 + 1];
        struct
        {
            int32 dw0;
            int32 dw1;
            int32 dw2;
        } Dw;
    } VendorResult;

    int Args[4];
    __cpuid(Args, 0);

    VendorResult.Dw.dw0 = Args[1];
    VendorResult.Dw.dw1 = Args[3];
    VendorResult.Dw.dw2 = Args[2];
    VendorResult.Buffer[12] = 0;

    std::memcpy(OutBuffer, VendorResult.Buffer, std::size(OutBuffer));
}

static void GetCPUBrand(char (&OutBrandString)[0x40])
{
    // @see for more information http://msdn.microsoft.com/en-us/library/vstudio/hskdteyh(v=vs.100).aspx
    char BrandString[0x40] = {0};
    int32 CPUInfo[4] = {-1};
    const size_t CPUInfoSize = sizeof(CPUInfo);

    __cpuid(CPUInfo, 0x80000000);
    const uint32 MaxExtIDs = CPUInfo[0];

    if (MaxExtIDs >= 0x80000004)
    {
        const uint32 FirstBrandString = 0x80000002;
        const uint32 NumBrandStrings = 3;
        for (uint32 Index = 0; Index < NumBrandStrings; Index++)
        {
            __cpuid(CPUInfo, FirstBrandString + Index);
            std::memcpy(BrandString + CPUInfoSize * Index, CPUInfo, CPUInfoSize);
        }
    }

    std::memcpy(OutBrandString, BrandString, std::size(BrandString));
}

bool SupportAES()
{
    int Reg[4];
    __cpuid(Reg, 1);
    return (Reg[2] & 0x02000000) != 0;    // Check if AES is supported
}

bool SupportAVX512()
{
    int Reg[4];
    __cpuidex(Reg, 7, 0);
    return (Reg[1] & (1 << 16)) != 0;    // Check if AVX512F is supported
}

bool SupportAVX2()
{
    int Reg[4];
    __cpuidex(Reg, 7, 0);
    return (Reg[1] & (1 << 5)) != 0;    // AVX2 is bit 5 of EBX
}

const FCPUInformation& FWindowsMisc::GetCPUInformation()
{
    static FCPUInformation Informations = {};
    if (Informations.Vendor[0] != '\0') [[likely]]
    {
        return Informations;
    }

    std::memset(&Informations, 0, sizeof(Informations));

    GetCPUBrand(Informations.Brand);
    GetCPUVendor(Informations.Vendor);

    Informations.AES = SupportAES();
    Informations.AVX2 = SupportAVX2();
    Informations.AVX512 = SupportAVX512();
    return Informations;
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

    new (TargetMemory) FMiMalloc;
    return true;
}

Ref<IExternalModule> FWindowsMisc::LoadExternalModule(const std::string& ModuleName)
{
    WeakRef<RWindowsExternalModule>* Iter = s_ModuleStorage.Find(ModuleName);

    if (Iter == nullptr || !Iter->IsValid())
    {
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
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &wAppDataPath)))
    {
        returnPath = std::filesystem::path(wAppDataPath);
        CoTaskMemFree(wAppDataPath);
    }
#endif

    return returnPath;
}
