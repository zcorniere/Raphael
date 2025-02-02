#pragma once

#include <cpplogger/Logger.hpp>

// IWYU pragma: begin_keep
#include <cstring>
#include <filesystem>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
// IWYU pragma: end_keep

#include <Engine/Core/RTTI.hxx>
#include <Engine/Misc/MiscDefines.hxx>

#include <Engine/Core/Memory/Memory.hxx>
#include <Engine/Core/RObject.hxx>

#include <Engine/Compilers/Compiler.hxx>

#include <Engine/Misc/Profiler.hxx>

#include <Engine/Misc/Assertions.hxx>
#include <Engine/Misc/Hash.hxx>

#include <Engine/Platforms/Platform.hxx>

#include <Engine/Math/Matrix.hxx>
#include <Engine/Math/Vector.hxx>

#include <Engine/Math/Math.hxx>

#include <Engine/Containers/Array.hxx>
#include <Engine/Containers/ArrayView.hxx>

// Make sure we're 64 bit
static_assert(sizeof(void*) == 8);

DECLARE_LOGGER_CATEGORY(Core, LogCore, Trace);

extern IMallocInterface* GMalloc;

extern uint64 GFrameCounter;
