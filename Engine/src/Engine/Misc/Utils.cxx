#include "Engine/Misc/Utils.hxx"

#include <fstream>

namespace Utils
{

std::string readFile(const std::filesystem::path& filename)
{
    /// Must be opened in binary mode, so Windows won't mess with the newlines
    std::string fileContent;
    std::ifstream file(filename, std::ios::binary);
    size_t fileSize = std::filesystem::file_size(filename);

    if (!file.is_open())
        return "";

    fileContent.resize(fileSize);
    file.read(fileContent.data(), fileSize);
    file.close();
    return fileContent;
}

#if defined(PLATFORM_WINDOWS)
    #define SPRINTF sprintf_s
#else
    #define SPRINTF sprintf
#endif

std::string BytesToString(uint64 bytes)
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

    return std::string(buffer);
}

#undef SPRINTF

}    // namespace Utils
