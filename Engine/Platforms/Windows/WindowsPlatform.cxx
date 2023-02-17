#include "Engine/Platforms/Windows/WindowsPlatform.hxx"

#include <debugapi.h>
#include <processthreadsapi.h>

DECLARE_LOGGER_CATEGORY(Core, LogWindowsPlateform, Info)

bool WindowsPlatform::isDebuggerPresent()
{
    return !!::IsDebuggerPresent();
}

void WindowsPlatform::setThreadName(std::jthread &thread, const std::string &name)
{
    std::wstring nameStupidType(name.begin(), name.end());
    HRESULT hr = ::SetThreadDescription(thread.native_handle(), nameStupidType.c_str());
    if (FAILED(hr)) { LOG(LogWindowsPlateform, Error, "SetThreadDescription('{}') failed", name); }
}

static std::string getThreadName(std::jthread &thread)
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
