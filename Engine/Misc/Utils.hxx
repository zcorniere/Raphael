#pragma once

#include <fstream>
#include <span>

namespace Utils
{

/// Read a whole file into a vector of byte.
template <typename T = std::byte>
std::vector<T> readBinaryFile(const std::filesystem::path &filename)
{
    size_t fileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::binary);
    std::vector<T> fileContent(fileSize / sizeof(T));

    if (!file.is_open()) throw std::runtime_error("failed to open file " + filename.string());
    file.read(reinterpret_cast<char *>(fileContent.data()), fileSize * sizeof(T));
    file.close();
    return fileContent;
}

/// Read a whole file into a string
std::string readFile(const std::filesystem::path &filename);

template <typename T = std::byte>
std::size_t writeBinaryFile(const std::filesystem::path &filename, const std::span<const T> &code)
{
    std::ofstream file(filename, std::ios::binary);
    file.write(reinterpret_cast<const char *>(code.data()), code.size_bytes());
    file.close();
    return code.size();
}

std::string BytesToString(uint64 Bytes);

}    // namespace Utils
