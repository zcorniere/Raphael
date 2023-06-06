project(RaphaelRHIVulkan)

add_subdirectory(External/)

option(ENABLE_VULKAN_DEBUGGING "" ON)

file(GLOB VULKAN_RENDERER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/VulkanRHI/*.cxx
     ${CMAKE_CURRENT_SOURCE_DIR}/src/VulkanRHI/Resources/*.cxx
)

add_library(${PROJECT_NAME} STATIC ${VULKAN_RENDERER_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_precompile_headers(${PROJECT_NAME} REUSE_FROM RaphaelEngine)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE spirv-cross-core
            shaderc
            VulkanMemoryAllocator
            VulkanHelpers
            Vulkan::Headers
)
target_link_libraries(${PROJECT_NAME} PUBLIC RaphaelEngine)

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

build_tests(
    ${PROJECT_NAME}
    tests/ShaderCompiler.cxx
)
# Give the tests access to the RHI headers
target_include_directories(${PROJECT_NAME}_Test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

