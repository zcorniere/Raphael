#include "Engine/Misc/StringUtils.hxx"

namespace Raphael::Utils
{

std::string BytesToString(uint64_t bytes)
{
    constexpr uint64_t GB = 1024 * 1024 * 1024;
    constexpr uint64_t MB = 1024 * 1024;
    constexpr uint64_t KB = 1024;

    char buffer[32];

    if (bytes > GB)
        sprintf(buffer, "%.2f GB", (float)bytes / (float)GB);
    else if (bytes > MB)
        sprintf(buffer, "%.2f MB", (float)bytes / (float)MB);
    else if (bytes > KB)
        sprintf(buffer, "%.2f KB", (float)bytes / (float)KB);
    else
        sprintf(buffer, "%.2f bytes", (float)bytes);

    return std::string(buffer);
}

}    // namespace Raphael::Utils
