#include "StreamWriter.hxx"

void StreamWriter::WriteString(const std::string& String)
{
    size_t Size = String.size();
    WriteData((uint8*)&Size, sizeof(size_t));
    WriteData((uint8*)String.data(), sizeof(char) * Size);
}
