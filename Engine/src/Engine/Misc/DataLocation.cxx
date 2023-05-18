#include "Engine/Misc/DataLocation.hxx"

#include "Engine/Platforms/PlatformMisc.hxx"

namespace fs = std::filesystem;

fs::path DataLocationFinder::GetShaderPath()
{
    fs::path ExePath = Platform::GetExecutablePath();

    fs::path SearchDirectory = ExePath;
    // Search all the way to the root. (add a limit ?)
    while (SearchDirectory.parent_path() != SearchDirectory) {
        fs::path FolderPath = SearchDirectory / "Shaders";
        if (fs::exists(FolderPath) && fs::is_directory(FolderPath)) { return FolderPath; }
        SearchDirectory = SearchDirectory.parent_path();
    }
    return fs::current_path();
}

fs::path DataLocationFinder::GetConfigPath()
{
#ifndef NDEBUG
    return fs::current_path();
#else
    return PlatformMisc::GetConfigPath();
#endif
}
