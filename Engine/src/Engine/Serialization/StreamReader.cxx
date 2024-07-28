#include "Engine/Serialization/StreamReader.hxx"

void StreamReader::ReadString(std::string& String)
{
    size_t Size = String.size();
    ReadData((uint8*)&Size, sizeof(size_t));

    String.resize(Size);
    ReadData((uint8*)String.data(), sizeof(char) * Size);
}
