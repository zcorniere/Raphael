project(RaphaelEngine)

if(WIN32)
    file(GLOB PLATFORM_SOURCE_FILES Platforms/Windows/*.cxx)
elseif(UNIX)
    file(GLOB PLATFORM_SOURCE_FILES Platforms/Unix/*.cxx)
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

file(GLOB VULKAN_RENDERER_FILES Renderer/Vulkan/*.cxx)

option(NO_PROFILING "Disable the profiling" OFF)

add_library(
    ${PROJECT_NAME} STATIC
    Core/Application.cxx
    Core/RObject.cxx
    Core/Log.cxx
    Misc/StringUtils.cxx
    Misc/Assertions.cxx
    Renderer/Renderer.cxx
    Renderer/RendererContext.cxx
    Threading/Thread.cxx
    Threading/ThreadPool.cxx
    ${PLATFORM_SOURCE_FILES}
    ${COMPILER_SOURCE_FILE}
    ${VULKAN_RENDERER_FILES}
)
target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<CONFIG:Debug>:RPH_DEBUG> RPH_LOG_ALLOCATION)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

target_link_libraries(${PROJECT_NAME} PUBLIC logger glm glfw VulkanMemoryAllocator Vulkan::Vulkan )
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
            /wd4324>
)

if(NO_PROFILING)
    target_link_libraries(${PROJECT_NAME} PUBLIC Tracy::TracyClient)
endif(NO_PROFILING)
