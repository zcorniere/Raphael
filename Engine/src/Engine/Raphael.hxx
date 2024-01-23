#pragma once

#include <cpplogger/Logger.hpp>
#include <glm/glm.hpp>

// IWYU pragma: begin_keep
#include <cstring>
#include <filesystem>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
// IWYU pragma: end_keep

#include <Engine/Misc/MiscDefines.hxx>

#include <Engine/Core/Memory/Memory.hxx>
#include <Engine/Core/RObject.hxx>

#include <Engine/Compilers/Compiler.hxx>

#include <Engine/Misc/Profiler.hxx>

#include <Engine/Misc/Assertions.hxx>
#include <Engine/Misc/Hash.hxx>

#include <Engine/Platforms/Platform.hxx>

#include <Engine/Containers/Array.hxx>

// Make sure we're 64 bit
static_assert(sizeof(void*) == 8);

DECLARE_LOGGER_CATEGORY(Core, LogCore, Trace);

extern Malloc* GMalloc;

extern uint64 GFrameCounter;
