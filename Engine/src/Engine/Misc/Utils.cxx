#include "Engine/Misc/Utils.hxx"

#include <fstream>

namespace Utils
{

String readFile(const std::filesystem::path& filename)
{
    /// Must be opened in binary mode, so Windows won't mess with the newlines
    String fileContent;
    std::ifstream file(filename, std::ios::binary);
    size_t fileSize = std::filesystem::file_size(filename);

    if (!file.is_open())
        return "";

    fileContent.Resize(fileSize);
    file.read(fileContent.Raw(), fileSize);
    file.close();
    return fileContent;
}

#if defined(PLATFORM_WINDOWS)
    #define SPRINTF sprintf_s
#else
    #define SPRINTF sprintf
#endif

String BytesToString(uint64 bytes)
{
    constexpr uint64_t GB = 1024 * 1024 * 1024;
    constexpr uint64_t MB = 1024 * 1024;
    constexpr uint64_t KB = 1024;

    char buffer[32];

    if (bytes > GB)
        SPRINTF(buffer, "%.2f GB", (float)bytes / (float)GB);
    else if (bytes > MB)
        SPRINTF(buffer, "%.2f MB", (float)bytes / (float)MB);
    else if (bytes > KB)
        SPRINTF(buffer, "%.2f KB", (float)bytes / (float)KB);
    else
        SPRINTF(buffer, "%.2f bytes", (float)bytes);

    return String(buffer);
}

#undef SPRINTF

static struct {
    bool bHasRequested = false;
    int ExitStatus = 0;
} GExitRequestInfo;
void RequestExit(int Status, bool bForce)
{
    if (bForce) {
        std::exit(Status);
    }
    GExitRequestInfo.ExitStatus = Status;
    GExitRequestInfo.bHasRequested = true;
}

bool HasRequestedExit(int& OutStatus)
{
    OutStatus = GExitRequestInfo.ExitStatus;
    return GExitRequestInfo.bHasRequested;
}

}    // namespace Utils
