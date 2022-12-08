#pragma once

#include <filesystem>
#include <source_location>

namespace raphael::utils
{

std::string function_name(const std::source_location &location = std::source_location::current())
{
    return location.function_name();
}

std::string file_position(const std::source_location &location = std::source_location::current())
{
    return std::string() +
           ::std::filesystem::relative(location.file_name(), ::std::filesystem::current_path()).string() + ":" +
           std::to_string(location.line()) + ":" + std::to_string(location.column());
}

}    // namespace pivot::utils
