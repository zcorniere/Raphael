# logger
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/logger-cpp)
target_compile_definitions(logger PUBLIC LOGGER_EXTERN_DECLARATION)

# magic_enum
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/magic_enum)

# Vulkan Header
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Vulkan-Headers)

# glm
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glm)
target_compile_definitions(glm INTERFACE GLM_FORCE_DEPTH_ZERO_TO_ONE)

# sdl
set(SDL_TEST_ENABLED_BY_DEFAULT
    OFF
    CACHE BOOL ""
)
set(SDL_OPENGL
    OFF
    CACHE BOOL ""
)
set(SDL_OPENGLES
    OFF
    CACHE BOOL ""
)
set(SDL_OSS
    OFF
    CACHE BOOL ""
)
set(SDL_ALSA
    OFF
    CACHE BOOL ""
)
set(SDL_ALSA_SHARED
    OFF
    CACHE BOOL ""
)
set(SDL_JACK
    OFF
    CACHE BOOL ""
)
set(SDL_JACK_SHARED
    OFF
    CACHE BOOL ""
)
set(SDL_PIPEWIRE
    OFF
    CACHE BOOL ""
)
set(SDL_PIPEWIRE_SHARED
    OFF
    CACHE BOOL ""
)
set(SDL_PULSEAUDIO
    OFF
    CACHE BOOL ""
)
set(SDL_PULSEAUDIO_SHARED
    OFF
    CACHE BOOL ""
)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/SDL)

# tracy
set(TRACY_ENABLE
    ON
    CACHE BOOL ""
)
set(TRACY_ON_DEMAND
    ON
    CACHE BOOL ""
)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tracy)

# vma
set(VMA_STATIC_VULKAN_FUNCTIONS
    OFF
    CACHE BOOL ""
)
set(VMA_DYNAMIC_VULKAN_FUNCTIONS
    ON
    CACHE BOOL ""
)
set(VMA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator)
add_library(VulkanMemoryAllocator STATIC ${VMA_PATH}/include/vk_mem_alloc.h)
set_source_files_properties(
    ${VMA_PATH}/include/vk_mem_alloc.h PROPERTIES
    LANGUAGE CXX
)
target_compile_definitions(VulkanMemoryAllocator PRIVATE VMA_IMPLEMENTATION
                                                 PRIVATE VMA_STATIC_VULKAN_FUNCTIONS=0
                                                 PRIVATE VMA_DYNAMIC_VULKAN_FUNCTIONS=1
)
target_include_directories(VulkanMemoryAllocator PUBLIC ${VMA_PATH}/include)
set_target_properties(VulkanMemoryAllocator PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(VulkanMemoryAllocator PUBLIC Vulkan::Headers)

# glslang
set(ENABLE_CTEST
    OFF
    CACHE BOOL "" FORCE
)
set(BUILD_EXTERNAL
    OFF
    CACHE BOOL "" FORCE
)
set(ENABLE_GLSLANG_BINARIES
    OFF
    CACHE BOOL "" FORCE
)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glslang/)

# SPIRV-Headers
set(SPIRV_HEADERS_SKIP_EXAMPLES
    ON
    CACHE BOOL "" FORCE
)
set(SPIRV_HEADERS_SKIP_INSTALL
    ON
    CACHE BOOL "" FORCE
)
set(SKIP_GLSLANG_INSTALL
    ON
    CACHE BOOL "" FORCE
)
set(ENABLE_GLSLANG_BINARIES
    OFF
    CACHE BOOL "" FORCE
)
set(ENABLE_SPVREMAPPER
    OFF
    CACHE BOOL "" FORCE
)
set(ENABLE_RTTI
    ON
    CACHE BOOL "" FORCE
)
set(ENABLE_OPT
    ON
    CACHE BOOL "" FORCE
)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/SPIRV-Headers/)

# SPIRV-Tools
set(SPIRV-Headers_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/SPIRV-Headers/"
    CACHE STRING "SPIRV-Headers path"
)
set(ENABLE_RTTI
    ON
    CACHE BOOL "" FORCE
)
set(SPIRV_SKIP_TESTS
    ON
    CACHE BOOL "" FORCE
)
set(SKIP_SPIRV_TOOLS_INSTALL
    ON
    CACHE BOOL "" FORCE
)
set(SPIRV_SKIP_EXECUTABLES
    ON
    CACHE BOOL "" FORCE
)
set(SPIRV_WERROR
    OFF
    CACHE BOOL "" FORCE
)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/SPIRV-Tools)

# shaderc
set(SHADERC_SKIP_TESTS
    ON
    CACHE BOOL ""
)
set(SHADERC_SKIP_INSTALL
    ON
    CACHE BOOL ""
)
set(SHADERC_SKIP_EXAMPLES
    ON
    CACHE BOOL ""
)
set(SHADERC_SKIP_COPYRIGHT_CHECK
    ON
    CACHE BOOL ""
)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/shaderc/)

# SPIRV-Cross
set(SPIRV_CROSS_ENABLE_TESTS
    OFF
    CACHE BOOL ""
)
set(SPIRV_CROSS_CLI
    OFF
    CACHE BOOL ""
)
set(SPIRV_CROSS_ENABLE_GLSL
    OFF
    CACHE BOOL ""
)
set(SPIRV_CROSS_ENABLE_HLSL
    OFF
    CACHE BOOL ""
)
set(SPIRV_CROSS_ENABLE_MSL
    OFF
    CACHE BOOL ""
)
set(SPIRV_CROSS_ENABLE_CPP
    OFF
    CACHE BOOL ""
)
set(SPIRV_CROSS_ENABLE_REFLECT
    OFF
    CACHE BOOL ""
)
set(SPIRV_CROSS_ENABLE_C_API
    OFF
    CACHE BOOL ""
)
set(SPIRV_CROSS_ENABLE_UTIL
    OFF
    CACHE BOOL ""
)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/SPIRV-Cross/)
