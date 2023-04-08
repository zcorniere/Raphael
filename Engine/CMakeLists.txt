project(RaphaelEngine)

option(ENABLE_VULKAN_DEBUGGING "" ON)

if(WIN32)
    file(GLOB PLATFORM_SOURCE_FILES Platforms/Windows/*.cxx)
    file(GLOB RENDERER_PLATFORM_SOURCE_FILES Renderer/Vulkan/Windows/*.cxx)
elseif(UNIX)
    file(GLOB PLATFORM_SOURCE_FILES Platforms/Linux/*.cxx)
    file(GLOB RENDERER_PLATFORM_SOURCE_FILES Renderer/Vulkan/Linux/*.cxx)
else()
    message(FATAL_ERROR "Platform Not supported !")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(COMPILER_SOURCE_FILE Compilers/ClangCompiler.cxx)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(COMPILER_SOURCE_FILE Compilers/GNUCompiler.cxx)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(COMPILER_SOURCE_FILE Compilers/MSVCCompiler.cxx)
else()
    message(FATAL_ERROR "Compiler not supported !")
endif()

file(GLOB VULKAN_RENDERER_FILES Renderer/Vulkan/*.cxx Renderer/Vulkan/Resources/*.cxx)

add_library(
    ${PROJECT_NAME} STATIC
    Core/RObject.cxx
    Core/Log.cxx

    Renderer/RHI/GenericRHI.cxx
    Renderer/RHI/RHICommandQueue.cxx

    Misc/Utils.cxx
    Misc/Assertions.cxx

    Threading/Thread.cxx
    Threading/ThreadPool.cxx
    
    ${PLATFORM_SOURCE_FILES}
    ${COMPILER_SOURCE_FILE}
    ${VULKAN_RENDERER_FILES}
    ${RENDERER_PLATFORM_SOURCE_FILES}
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC logger
           magic_enum
           glm
           SDL3
           spirv-cross-core
           shaderc
           VulkanMemoryAllocator
           Vulkan::Headers
           VulkanHelpers
)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
target_precompile_headers(${PROJECT_NAME} PRIVATE Raphael.hxx)

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

if(ENABLE_VULKAN_DEBUGGING)
    target_compile_definitions(${PROJECT_NAME} PUBLIC VULKAN_DEBUGGING_ENABLED=1)
endif()
