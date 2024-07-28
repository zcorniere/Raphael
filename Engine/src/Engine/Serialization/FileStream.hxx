#pragma once

#include "Engine/Serialization/StreamReader.hxx"
#include "Engine/Serialization/StreamWriter.hxx"

#include <filesystem>
#include <fstream>

class FileStreamWriter : public StreamWriter
{
public:
    FileStreamWriter(const std::filesystem::path& Path);
    FileStreamWriter(const FileStreamWriter&) = delete;
    virtual ~FileStreamWriter();

    virtual bool IsGood() const override final;
    virtual uint64_t GetStreamPosition() override final;
    virtual void SetStreamPosition(uint64_t position) override final;
    virtual bool WriteData(const uint8* Data, size_t Size) override final;

    void Flush();

private:
    std::filesystem::path Path;
    std::ofstream File;
};

class FileStreamReader : public StreamReader
{

public:
    FileStreamReader(const std::filesystem::path& Path);
    FileStreamReader(const FileStreamReader&) = delete;
    virtual ~FileStreamReader();

    virtual bool IsGood() const override final;
    virtual uint64_t GetStreamPosition() override final;
    virtual void SetStreamPosition(uint64_t position) override final;
    virtual bool ReadData(uint8* Data, size_t Size) override final;

private:
    std::filesystem::path Path;
    std::ifstream File;
};
