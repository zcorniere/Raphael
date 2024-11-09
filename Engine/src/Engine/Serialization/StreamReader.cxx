#include "Engine/Serialization/StreamReader.hxx"

namespace Serialization
{

void FStreamReader::ReadString(std::string& String)
{
    uint32 Size;
    ReadData((uint8*)&Size, sizeof(uint32));

    String.resize(Size);
    ReadData((uint8*)String.data(), sizeof(char) * Size);
}

}    // namespace Serialization
