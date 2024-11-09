#pragma once

#include "Engine/Serialization/StreamReader.hxx"
#include "Engine/Serialization/StreamWriter.hxx"

#include <filesystem>
#include <fstream>

namespace Serialization
{
class FFileStreamWriter : public FStreamWriter
{
public:
    FFileStreamWriter(const std::filesystem::path& Path);
    FFileStreamWriter(const FFileStreamWriter&) = delete;
    virtual ~FFileStreamWriter();

    virtual bool IsGood() const override final;
    virtual uint64_t GetStreamPosition() override final;
    virtual void SetStreamPosition(uint64_t position) override final;
    virtual bool WriteData(const uint8* Data, size_t Size) override final;

    void Flush();

private:
    std::filesystem::path Path;
    std::ofstream File;
};

class FFileStreamReader : public FStreamReader
{

public:
    FFileStreamReader(const std::filesystem::path& Path);
    FFileStreamReader(const FFileStreamReader&) = delete;
    virtual ~FFileStreamReader();

    virtual bool IsGood() const override final;
    virtual uint64_t GetStreamPosition() override final;
    virtual void SetStreamPosition(uint64_t position) override final;
    virtual bool ReadData(uint8* Data, size_t Size) override final;

private:
    std::filesystem::path Path;
    std::ifstream File;
};

}    // namespace Serialization
