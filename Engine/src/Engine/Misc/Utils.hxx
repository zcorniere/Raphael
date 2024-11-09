#pragma once

#include <fstream>
#include <span>

namespace Utils
{

/// Read a whole file into a vector of byte.
template <typename T = std::byte>
bool ReadBinaryFile(const std::filesystem::path& filename, TArray<T>& FileContent)
{
    const size_t fileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::binary);

    FileContent.Resize(fileSize / sizeof(T));
    if (!file.is_open())
        return {};
    file.read(reinterpret_cast<char*>(FileContent.data()), fileSize * sizeof(T));
    file.close();
    return FileContent;
}

/// Read a whole file into a string
std::string ReadFile(const std::filesystem::path& filename);

template <typename T = std::byte>
std::size_t WriteBinaryFile(const std::filesystem::path& filename, const std::span<const T>& code)
{
    std::ofstream file(filename, std::ios::binary);
    file.write(reinterpret_cast<const char*>(code.data()), code.size_bytes());
    file.close();
    return code.size();
}

std::string BytesToString(uint64 Bytes);

void RequestExit(int InStatus, bool bForce = false);
bool HasRequestedExit(int& OutStatus);

}    // namespace Utils
