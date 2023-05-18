#pragma once

class DataLocationFinder
{
public:
    static std::filesystem::path GetShaderPath();
    static std::filesystem::path GetConfigPath();
};
