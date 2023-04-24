#include "Engine/Misc/DataLocation.hxx"

std::filesystem::path DataLocationFinder::GetShaderPath()
{
    namespace fs = std::filesystem;

    fs::path ExePath = Platform::GetExecutablePath();

    fs::path SearchDirectory = ExePath;
    // Search all the way to the root. (add a limit ?)
    while (SearchDirectory.parent_path() != SearchDirectory) {
        fs::path FolderPath = SearchDirectory / "Shaders";
        if (fs::exists(FolderPath) && fs::is_directory(FolderPath)) { return FolderPath; }
        SearchDirectory = SearchDirectory.parent_path();
    }
    checkNoEntry();
}
