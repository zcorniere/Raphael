# logger
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/logger-cpp)
target_compile_definitions(logger PUBLIC LOGGER_EXTERN_DECLARATION)

# Vulkan Header
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Vulkan-Headers)

# glm
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glm)
target_compile_definitions(glm INTERFACE GLM_FORCE_DEPTH_ZERO_TO_ONE)

# sdl
set(SDL_TEST_ENABLED_BY_DEFAULT OFF CACHE BOOL "")
set(SDL_OPENGL OFF CACHE BOOL "")
set(SDL_OPENGLES OFF CACHE BOOL "")
set(SDL_OSS OFF CACHE BOOL "")
set(SDL_ALSA OFF CACHE BOOL "")
set(SDL_ALSA_SHARED OFF CACHE BOOL "")
set(SDL_JACK OFF CACHE BOOL "")
set(SDL_JACK_SHARED OFF CACHE BOOL "")
set(SDL_PIPEWIRE OFF CACHE BOOL "")
set(SDL_PIPEWIRE_SHARED OFF CACHE BOOL "")
set(SDL_PULSEAUDIO OFF CACHE BOOL "")
set(SDL_PULSEAUDIO_SHARED OFF CACHE BOOL "")
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
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator)
target_compile_features(VulkanMemoryAllocator PUBLIC cxx_std_23)
