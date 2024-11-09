#include "Engine/Serialization/FileStream.hxx"

namespace Serialization
{

FFileStreamWriter::FFileStreamWriter(const std::filesystem::path& Path): Path(Path)
{
    File.open(Path, std::ios::binary | std::ios::out);
}

FFileStreamWriter::~FFileStreamWriter()
{
    File.close();
}

bool FFileStreamWriter::IsGood() const
{
    return File.good();
}

uint64_t FFileStreamWriter::GetStreamPosition()
{
    return File.tellp();
}

void FFileStreamWriter::SetStreamPosition(uint64_t position)
{
    File.seekp(position);
}

bool FFileStreamWriter::WriteData(const uint8* Data, size_t Size)
{
    File.write(reinterpret_cast<const char*>(Data), Size);
    return true;
}

void FFileStreamWriter::Flush()
{
    File.flush();
}

//
// ===============================================
//

FFileStreamReader::FFileStreamReader(const std::filesystem::path& Path): Path(Path)
{
    File.open(Path, std::ios::binary | std::ios::in);
}

FFileStreamReader::~FFileStreamReader()
{
    File.close();
}

bool FFileStreamReader::IsGood() const
{
    return File.good();
}

uint64_t FFileStreamReader::GetStreamPosition()
{
    return File.tellg();
}

void FFileStreamReader::SetStreamPosition(uint64_t position)
{
    File.seekg(position);
}

bool FFileStreamReader::ReadData(uint8* Data, size_t Size)
{
    File.read(reinterpret_cast<char*>(Data), Size);
    return true;
}

}    // namespace Serialization
