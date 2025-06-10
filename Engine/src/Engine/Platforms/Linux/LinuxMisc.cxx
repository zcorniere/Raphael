#include "Engine/Platforms/Linux/LinuxMisc.hxx"

#include "Engine/Core/Window.hxx"

#include "Engine/Core/Memory/MiMalloc.hxx"
#include "Engine/Core/Memory/StdMalloc.hxx"
#include "Engine/Misc/Assertions.hxx"
#include "Engine/Misc/CommandLine.hxx"

#include <ModernDialogs.h>
#include <cpuid.h>
#include <dlfcn.h>
#include <filesystem>

#define XDG_NO_EXCEPTION
#include <xdg.hpp>

EBoxReturnType FLinuxMisc::DisplayMessageBox(EBoxMessageType MsgType, const std::string Title, const std::string Text)
{
    if (!RWindow::EnsureGLFWInit())
    {
        return FGenericMisc::DisplayMessageBox(MsgType, Title, Text);
    }

    MD::Style Style = MD::Style::Error;
    MD::Buttons Button = MD::Buttons::OK;

    switch (MsgType)
    {
        case EBoxMessageType::Ok:
            Button = MD::Buttons::OK;
            break;
        case EBoxMessageType::YesNo:
            Button = MD::Buttons::YesNo;
            break;
    }

    MD::Selection Result = MD::ShowMsgBox(Title, Text, Style, Button);

    switch (Result)
    {
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

static void GetCPUVendor(char (&OutBuffer)[12 + 1])
{
    std::memset(OutBuffer, 0, sizeof(OutBuffer));

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

    int32 Args[4];
    __cpuid(0, Args[0], Args[1], Args[2], Args[3]);

    VendorResult.Dw.dw0 = Args[1];
    VendorResult.Dw.dw1 = Args[3];
    VendorResult.Dw.dw2 = Args[2];
    VendorResult.Buffer[12] = 0;

    std::memcpy(OutBuffer, VendorResult.Buffer, std::size(OutBuffer));
}

static void GetCPUBrand(char (&OutBrandString)[0x40])
{
    std::memset(OutBrandString, 0, sizeof(OutBrandString));

    // @see for more information http://msdn.microsoft.com/en-us/library/vstudio/hskdteyh(v=vs.100).aspx
    char BrandString[0x40] = {0};
    int32 CPUInfo[4] = {-1};
    const size_t CPUInfoSize = sizeof(CPUInfo);

    __cpuid(0x80000000, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);
    const uint32 MaxExtIDs = CPUInfo[0];

    if (MaxExtIDs >= 0x80000004)
    {
        const uint32 FirstBrandString = 0x80000002;
        const uint32 NumBrandStrings = 3;
        for (uint32 Index = 0; Index < NumBrandStrings; Index++)
        {
            __cpuid(FirstBrandString + Index, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);
            std::memcpy(BrandString + CPUInfoSize * Index, CPUInfo, CPUInfoSize);
        }
    }

    std::memcpy(OutBrandString, BrandString, std::size(BrandString));
}

bool SupportAES()
{
    unsigned int eax, ebx, ecx, edx;
    __cpuid(1, eax, ebx, ecx, edx);
    return (ecx & 0x02000000) != 0;    // Check if AES is supported
}

bool SupportAVX512()
{
    unsigned int eax, ebx, ecx, edx;
    __cpuid_count(7, 0, eax, ebx, ecx, edx);
    return (ebx & (1 << 16)) != 0;    // Check if AVX512F is supported
}

bool SupportAVX2()
{
    unsigned int eax, ebx, ecx, edx;
    __cpuid_count(7, 0, eax, ebx, ecx, edx);
    return (ebx & (1 << 5)) != 0;    // AVX2 is bit 5 of EBX
}

const FCPUInformation& FLinuxMisc::GetCPUInformation()
{
    static FCPUInformation Info;
    if (Info.Vendor[0] != '\0')
    {
        return Info;
    }

    GetCPUBrand(Info.Brand);
    GetCPUVendor(Info.Vendor);

    Info.AVX512 = SupportAVX512();
    Info.AVX2 = SupportAVX2();
    Info.AES = SupportAES();

    return Info;
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

    if (FCommandLine::Param("usemimalloc"))
    {
        new (TargetMemory) FMiMalloc;
    }
    else
    {
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
