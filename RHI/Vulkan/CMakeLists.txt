project(RaphaelRHIVulkan)

option(ENABLE_VULKAN_DEBUGGING "" ON)

file(GLOB VULKAN_RENDERER_FILES
     ${CMAKE_CURRENT_SOURCE_DIR}/*.cxx
     ${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.cxx
)

add_library(${PROJECT_NAME} STATIC ${VULKAN_RENDERER_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
target_precompile_headers(${PROJECT_NAME} REUSE_FROM RaphaelEngine)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC RaphaelEngine
           spirv-cross-core
           shaderc
           VulkanMemoryAllocator
           Vulkan::Headers
           VulkanHelpers
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

if(ENABLE_VULKAN_DEBUGGING)
    target_compile_definitions(${PROJECT_NAME} PUBLIC VULKAN_DEBUGGING_ENABLED=1)
endif()
