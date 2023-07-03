#pragma once

#include <cpplogger/Logger.hpp>
#include <glm/glm.hpp>

#include <cstring>
#include <filesystem>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include <Engine/Misc/Profiler.hxx>

#include <Engine/Compilers/Compiler.hxx>
#include <Engine/Core/RObject.hxx>

#include <Engine/Misc/Assertions.hxx>
#include <Engine/Misc/Hash.hxx>
#include <Engine/Misc/Memory.hxx>
#include <Engine/Misc/MiscDefines.hxx>

#include <Engine/Platforms/Platform.hxx>

#include <Engine/Containers/Array.hxx>

static_assert(sizeof(void*) == 8);

DECLARE_LOGGER_CATEGORY(Core, LogCore, Trace);
