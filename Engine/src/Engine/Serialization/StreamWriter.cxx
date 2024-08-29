#include "StreamWriter.hxx"

namespace Serialization
{

void StreamWriter::WriteString(const std::string_view& String)
{
    uint32 Size = String.size();
    WriteData((uint8*)&Size, sizeof(uint32));
    WriteData((uint8*)String.data(), sizeof(char) * Size);
}

}    // namespace Serialization
