cmake_minimum_required(VERSION 3.20)
project(Raphael)

include(FetchContent)
include(CheckIPOSupported)
include(GNUInstallDirs)
include(cmake/find_linker.cmake)
include(cmake/CompileShaders.cmake)

find_linker()

set(CMAKE_CXX_STANDARD 23)
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

add_subdirectory(Engine/)