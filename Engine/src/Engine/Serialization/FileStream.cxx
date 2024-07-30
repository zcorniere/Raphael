#include "Engine/Serialization/FileStream.hxx"

namespace Serialization
{

FileStreamWriter::FileStreamWriter(const std::filesystem::path& Path): Path(Path)
{
    File.open(Path, std::ios::binary | std::ios::out);
}

FileStreamWriter::~FileStreamWriter()
{
    File.close();
}

bool FileStreamWriter::IsGood() const
{
    return File.good();
}

uint64_t FileStreamWriter::GetStreamPosition()
{
    return File.tellp();
}

void FileStreamWriter::SetStreamPosition(uint64_t position)
{
    File.seekp(position);
}

bool FileStreamWriter::WriteData(const uint8* Data, size_t Size)
{
    File.write(reinterpret_cast<const char*>(Data), Size);
    return true;
}

void FileStreamWriter::Flush()
{
    File.flush();
}

//
// ===============================================
//

FileStreamReader::FileStreamReader(const std::filesystem::path& Path): Path(Path)
{
    File.open(Path, std::ios::binary | std::ios::in);
}

FileStreamReader::~FileStreamReader()
{
    File.close();
}

bool FileStreamReader::IsGood() const
{
    return File.good();
}

uint64_t FileStreamReader::GetStreamPosition()
{
    return File.tellg();
}

void FileStreamReader::SetStreamPosition(uint64_t position)
{
    File.seekg(position);
}

bool FileStreamReader::ReadData(uint8* Data, size_t Size)
{
    File.read(reinterpret_cast<char*>(Data), Size);
    return true;
}

}    // namespace Serialization
