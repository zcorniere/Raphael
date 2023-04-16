project(RaphaelEngine)

set(SELECTED_RHI "Vulkan" CACHE STRING "Select the RHI to use")

if(WIN32)
    file(GLOB PLATFORM_SOURCE_FILES Platforms/Windows/*.cxx)
elseif(UNIX)
    file(GLOB PLATFORM_SOURCE_FILES Platforms/Linux/*.cxx)
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


add_library(
    ${PROJECT_NAME} STATIC
    Core/RObject.cxx
    Core/Log.cxx
    Core/Application.cxx
    Core/UUID.cxx

    Core/FrameGraph/FrameGraph.cxx
    Core/FrameGraph/PassNode.cxx
    Core/FrameGraph/FrameGraphBuilder.cxx
    Core/FrameGraph/ResourceEntry.cxx
    Core/FrameGraph/ResourceNode.cxx
    Core/Window.cxx
    Core/RHI/GenericRHI.cxx
    Core/RHI/RHICommandQueue.cxx

    Misc/Utils.cxx
    Misc/Assertions.cxx

    Threading/Thread.cxx
    Threading/ThreadPool.cxx
    
    ${PLATFORM_SOURCE_FILES}
    ${COMPILER_SOURCE_FILE}
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC logger
           magic_enum
           glm
           SDL3-static
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
target_precompile_headers(${PROJECT_NAME} PRIVATE Raphael.hxx)

if (SELECTED_RHI STREQUAL "Vulkan")
    target_link_libraries(${PROJECT_NAME} PUBLIC RaphaelRHIVulkan)
    target_compile_definitions(${PROJECT_NAME} PUBLIC RAPHAEL_SELECTED_RHI_VULKAN=1)
else()
    message(Fatal "${SELECTED_RHI} is not supported !")
endif()
