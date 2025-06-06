#include "Engine/Platforms/Linux/LinuxPlatform.hxx"
#include "Engine/Misc/Assertions.hxx"
#include "Engine/Misc/Utils.hxx"

#include <cstring>
#include <dlfcn.h>
#include <execinfo.h>
#include <fcntl.h>
#include <iterator>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

DECLARE_LOGGER_CATEGORY(Core, LogUnixPlateform, Info)

void FLinuxPlateform::Initialize()
{
    if (geteuid() == 0)
    {
        fprintf(stderr, "Refusing to run with the root privileges.\n");
        Utils::RequestExit(1, true);
    }
}

bool FLinuxPlateform::isDebuggerPresent()
{
    // If a process is tracing this one then TracerPid in /proc/self/status will
    // be the id of the tracing process.

    // Performance wise, the /proc filesystem is ram only, so it is ok

    int StatusFile = open("/proc/self/status", O_RDONLY);
    if (StatusFile == -1) [[unlikely]]
    {
        // Failed - unknown debugger status.
        return false;
    }

    char Buffer[256] = {0};
    const ssize_t Length = read(StatusFile, Buffer, sizeof(Buffer) - 1);
    if (Length == -1) [[unlikely]]
    {
        // Failed - unknown debugger status.
        return false;
    }

    close(StatusFile);

    constexpr char TracerString[] = "TracerPid:\t";
    const ssize_t LenTracerString = std::strlen(TracerString);

    const char* const foundStr = std::strstr(Buffer, TracerString);

    if (foundStr != nullptr) [[likely]]
    {
        return foundStr[LenTracerString] != '0';
    }
    else
    {
        return false;
    }
}

std::filesystem::path FLinuxPlateform::GetExecutablePath()
{
    return std::filesystem::canonical("/proc/self/exe");
}

void FLinuxPlateform::setThreadName(std::jthread& thread, const std::string& name)
{
    std::string sizeLimitedThreadName = name;

    if (sizeLimitedThreadName.size() > UnixThreadNameLimit)
    {
        constexpr char sDash[] = "-";
        constexpr size_t uDashLen = std::size(sDash);
        constexpr int numToLeave = (UnixThreadNameLimit - uDashLen) / 2;
        constexpr int delimiter = UnixThreadNameLimit - (numToLeave + uDashLen);

        sizeLimitedThreadName = name.substr(0, delimiter);
        sizeLimitedThreadName += sDash;
        sizeLimitedThreadName += name.substr(numToLeave + delimiter, name.size());

        check(sizeLimitedThreadName.size() <= UnixThreadNameLimit);
    }

    int errorCode = pthread_setname_np(thread.native_handle(), sizeLimitedThreadName.c_str());
    if (errorCode != 0)
    {
        LOG(LogUnixPlateform, Error, "pthread_setname_np(\"{}\") failed with error {} ({})", name, errorCode,
            strerror(errorCode));
    }
}

std::string FLinuxPlateform::getThreadName(std::jthread& thread)
{
    char name[UnixThreadNameLimit + 1] = {'\0'};

    int errorCode = pthread_getname_np(thread.native_handle(), name, std::size(name));
    if (errorCode != 0)
    {
        LOG(LogUnixPlateform, Error, "pthread_getname_np() failed with error {}({})", errorCode, strerror(errorCode));
    }
    return std::string(name);
}
