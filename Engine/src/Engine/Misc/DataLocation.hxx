#pragma once

/// @brief Helper class to find location of files, like config and shader
class DataLocationFinder
{
public:
    /// @brief Return the path to the folder containing the shaders
    static std::filesystem::path GetShaderPath();
    /// @brief Return the path to the config path
    static std::filesystem::path GetConfigPath();
};
