#include "Engine/Platforms/Windows/WindowsPlatform.hxx"

#include <windows.h>

#include <debugapi.h>
#include <libloaderapi.h>
#include <processthreadsapi.h>

#include <fcntl.h>
#include <io.h>
#include <iostream>
#include "WindowsPlatform.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogWindowsPlateform, Info)

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
}

void WindowsPlatform::Deinitialize()
{
    FreeConsole();
}

bool WindowsPlatform::isDebuggerPresent()
{
    return !!::IsDebuggerPresent();
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
