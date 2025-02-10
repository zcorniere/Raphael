project(RaphaelEngine)
set(CMAKE_FOLDER "Raphael/Engine")

set(SELECTED_RHI
    "Vulkan"
    CACHE STRING "Select the RHI to use"
)

add_library(RHI_Selector INTERFACE)
if(SELECTED_RHI STREQUAL "Vulkan")
    target_link_libraries(RHI_Selector INTERFACE RaphaelRHIVulkan)
    target_compile_definitions(RHI_Selector INTERFACE RAPHAEL_SELECTED_RHI_VULKAN=1)
else()
    message(FATAL_ERROR "${SELECTED_RHI} is not supported !")
endif()


add_subdirectory(External/)

if(WIN32)
    file(GLOB PLATFORM_SOURCE_FILES src/Engine/Platforms/Windows/*.cxx)
elseif(UNIX)
    file(GLOB PLATFORM_SOURCE_FILES src/Engine/Platforms/Linux/*.cxx)
else()
    message(FATAL_ERROR "Platform Not supported !")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(COMPILER_SOURCE_FILE src/Engine/Compilers/ClangCompiler.cxx)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(COMPILER_SOURCE_FILE src/Engine/Compilers/GNUCompiler.cxx)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(COMPILER_SOURCE_FILE src/Engine/Compilers/MSVCCompiler.cxx)
else()
    message(FATAL_ERROR "Compiler not supported !")
endif()

add_library(
    ${PROJECT_NAME} STATIC
    src/Engine/Main.cxx
    src/Engine/Core/Log.cxx
    src/Engine/Core/UUID.cxx
    src/Engine/Core/RObject.cxx
    src/Engine/Core/Engine.cxx
    src/Engine/Core/Application.cxx
    src/Engine/Core/Window.cxx
    src/Engine/Core/RHI/RHI.cxx
    src/Engine/Core/RHI/RHIDefinitions.cxx
    src/Engine/Core/RHI/RHICommandList.cxx
    src/Engine/Core/RHI/RHICommand.cxx
    src/Engine/Core/RHI/RHIScene.cxx
    src/Engine/Core/Memory/Memory.cxx
    src/Engine/Core/Memory/MiMalloc.cxx
    src/Engine/Core/Memory/StdMalloc.cxx
    src/Engine/Core/ECS/ComponentStorage.cxx
    src/Engine/Core/ECS/SystemScheduler.cxx
    src/Engine/Core/ECS/World.cxx
    src/Engine/Core/ECS/ECS.cxx
    src/Engine/AssetRegistry/AssetRegistry.cxx
    src/Engine/AssetRegistry/Asset.cxx
    src/Engine/AssetRegistry/Material.cxx
    src/Engine/AssetRegistry/MeshFactory.cxx
    src/Engine/Serialization/StreamWriter.cxx
    src/Engine/Serialization/StreamReader.cxx
    src/Engine/Serialization/FileStream.cxx
    src/Engine/Misc/DataLocation.cxx
    src/Engine/Misc/Timer.cxx
    src/Engine/Misc/Utils.cxx
    src/Engine/Misc/Assertions.cxx
    src/Engine/Misc/CommandLine.cxx
    src/Engine/Threading/Thread.cxx
    src/Engine/Threading/ThreadPool.cxx
    ${PLATFORM_SOURCE_FILES}
    ${COMPILER_SOURCE_FILE}
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC cpplogger
           RHI_Selector
           magic_enum
           glfw
           mimalloc-static
           ModernDialogs
)

target_compile_options(
    ${PROJECT_NAME}
    PRIVATE $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
            -Wall
            -Wextra
            -Wno-missing-field-initializers>
            $<$<CXX_COMPILER_ID:MSVC>:
            /Zc:preprocessor
            /W4
            /wd4267
            /wd4201
            /wd4244
            /wd4324
            /wd4702>
)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)
target_precompile_headers(${PROJECT_NAME} PRIVATE src/Engine/Raphael.hxx)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:RPH_POISON_ALLOCATION>)
target_compile_definitions(${PROJECT_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS)
target_disable_rtti(${PROJECT_NAME})

if(UNIX)
    target_link_libraries(${PROJECT_NAME} PUBLIC xdgpp)
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME} PUBLIC dbghelp)
endif()

build_tests(
    ${PROJECT_NAME}
    tests/Containers/Array.cxx
    tests/Containers/ArrayView.cxx
    tests/RHIShaderParametersTest.cxx
    tests/Math/Quaternion.cxx
    tests/Math/Vector.cxx
    tests/Math/Math.cxx
    tests/Math/Matrix.cxx
    tests/Math/Transform.cxx
    tests/Math/ViewPoint.cxx
    tests/RTTI.cxx
    tests/CommandLine.cxx
)
target_link_libraries(${PROJECT_NAME}_Test PRIVATE glm)

if(RPH_ENABLE_PROFILING)
    target_compile_definitions(${PROJECT_NAME} PUBLIC RPH_ENABLE_PROFILING=1)
    if(RPH_ENABLE_MEMORY_PROFILING)
        target_compile_definitions(${PROJECT_NAME} PUBLIC RPH_ENABLE_MEMORY_PROFILING=1)
    endif(RPH_ENABLE_MEMORY_PROFILING)
    target_link_libraries(${PROJECT_NAME} PUBLIC Tracy::TracyClient)
endif(RPH_ENABLE_PROFILING)

option(RPH_NAN_CHECKS "Enable NaN checks in the engine" ON)
if(RPH_NAN_CHECKS)
    message(STATUS "RPH - NaN checks enabled")
    target_compile_definitions(${PROJECT_NAME} PUBLIC RPH_NAN_CHECKS=1)
else()
    message(STATUS "RPH - NaN checks disabled")
    target_compile_definitions(${PROJECT_NAME} PUBLIC RPH_NAN_CHECKS=0)
endif(RPH_NAN_CHECKS)
