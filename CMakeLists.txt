cmake_minimum_required(VERSION 3.31)
project(Raphael)

option(BUILD_TESTING "Build the unit tests" ON)

include(CheckIPOSupported)
include(GNUInstallDirs)
include(cmake/tests.cmake)
include(cmake/find_linker.cmake)
include(cmake/CompileShaders.cmake)
include(cmake/StripDebugFile.cmake)
include(cmake/disable_rtti.cmake)

# Option to force colored output of the compiler
option(RPH_FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)
if(RPH_FORCE_COLORED_OUTPUT)
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options(-fdiagnostics-color=always)
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options(-fcolor-diagnostics)
    endif()
endif()

option(RPH_ENABLE_ASAN "Enable AddressSanitizer" OFF)
if (RPH_ENABLE_ASAN)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif(RPH_ENABLE_ASAN)

find_linker()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

check_ipo_supported(RESULT supported)
if(supported)
    message(STATUS "LTO enabled")
    cmake_policy(SET CMP0069 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
    set_property(GLOBAL PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "LTO not supported")
endif()

option(OPTIMIZE_FOR_NATIVE "Build with -march=native" ON)
if(OPTIMIZE_FOR_NATIVE)
    message(STATUS "Building with -march=native")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        message(WARNING "Native optimization is not supported for MSVC")
        set(OPTIMIZE_FOR_NATIVE OFF)
    endif()
else()
    message(STATUS "Building without -march=native")
endif()

option(RPH_BUILD_DOCUMENTATION "Build the Doxygen documentation" OFF)
if(RPH_BUILD_DOCUMENTATION)
    message(STATUS "Documentation building using Doxygen enabled")
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
        set(DOXYGEN_WARN_LOGFILE ${CMAKE_BINARY_DIR}/doxygen-warnings.log)
        set(DOXYGEN_PREDEFINED DOXYGEN_SHOULD_SKIP_THIS)
        doxygen_add_docs(doc Engine/src/ RHI/Vulkan/src/ README.md)
    else()
        message(SEND_ERROR "Doxygen needs to be installed to build the documentation")
    endif()
endif()

option(RPH_ENABLE_PROFILING "Enable the profiling" OFF)
option(RPH_ENABLE_MEMORY_PROFILING "Enable the memory profiling" OFF)
if(RPH_ENABLE_PROFILING)
    message(STATUS "RPH - Profiling is enabled")

    if(RPH_ENABLE_MEMORY_PROFILING)
        message(STATUS "RPH - Memory profiling is enabled")
    else()
        message(STATUS "RPH - Memory profiling is disabled")
    endif(RPH_ENABLE_MEMORY_PROFILING)
else()
    message(STATUS "RPH - Profiling is disabled")
endif(RPH_ENABLE_PROFILING)

add_subdirectory(External/)
add_subdirectory(Engine/)
add_subdirectory(Editor/)
add_subdirectory(RHI/Vulkan/)
