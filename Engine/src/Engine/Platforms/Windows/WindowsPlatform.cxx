#include "Engine/Platforms/Windows/WindowsPlatform.hxx"

#include <windows.h>

#include <debugapi.h>
#include <libloaderapi.h>
#include <processthreadsapi.h>
#include <dbghelp.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>

#include "WindowsPlatform.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogWindowsPlateform, Info)

static HANDLE GDebugSymbolHandle;

void WindowsPlatform::Initialize()
{
    // allocate a console for this app
    if (!AllocConsole())
    {
        return;
    }
    
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONIN$", "w", stdin);

    // Init the symbol loading system
    DWORD  error;
    HANDLE hCurrentProcess = GetCurrentProcess();

    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);

    if (!DuplicateHandle(hCurrentProcess, hCurrentProcess, hCurrentProcess, &GDebugSymbolHandle, 0, FALSE, DUPLICATE_SAME_ACCESS))
    {
        // DuplicateHandle failed
        error = GetLastError();
        fprintf(stderr, "DuplicateHandle returned error : %d\n", error);
        return;
    }

    if (!SymInitialize(GDebugSymbolHandle, NULL, TRUE))
    {
        // SymInitialize failed
        error = GetLastError();
       fprintf(stderr, "SymInitialize returned error : %d\n", error);
        return;
    }
}

void WindowsPlatform::Deinitialize()
{
    SymCleanup(GDebugSymbolHandle);
    FreeConsole();
}

bool WindowsPlatform::isDebuggerPresent()
{
    return ::IsDebuggerPresent();
}

std::filesystem::path WindowsPlatform::GetExecutablePath()
{
    char Buffer[MAX_PATH] = {0};
    GetModuleFileNameA(nullptr, Buffer, MAX_PATH);
    return std::filesystem::canonical(Buffer);
}

void WindowsPlatform::setThreadName(std::jthread& thread, const std::string& name)
{
    std::wstring nameStupidType(name.begin(), name.end());
    HRESULT hr = ::SetThreadDescription(thread.native_handle(), nameStupidType.c_str());
    if (FAILED(hr)) {
        LOG(LogWindowsPlateform, Error, "SetThreadDescription('{}') failed", name);
    }
}

std::string WindowsPlatform::getThreadName(std::jthread& thread)
{
    PWSTR name;
    HRESULT hr = ::GetThreadDescription(thread.native_handle(), &name);
    if (SUCCEEDED(hr)) {
        std::wstring nameNoStupidType(name, wcslen(name));
        LocalFree(name);
        return std::string(nameNoStupidType.begin(), nameNoStupidType.end());
    }
    return "";
}

void* WindowsPlatform::GetDebugSymbolHandle()
{
    return GDebugSymbolHandle;
}
