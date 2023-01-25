# logger
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/logger-cpp)
target_compile_definitions(logger PUBLIC LOGGER_EXTERN_DECLARATION)

# glm
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glm)
target_compile_definitions(glm INTERFACE GLM_FORCE_DEPTH_ZERO_TO_ONE)

# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_VULKAN_STATIC OFF CACHE BOOL "")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glfw)

# tracy
set(TRACY_ENABLE ON CACHE BOOL "")
set(TRACY_ON_DEMAND ON CACHE BOOL "")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tracy)

# vma
set(VMA_STATIC_VULKAN_FUNCTIONS OFF CACHE BOOL "")
set(VMA_DYNAMIC_VULKAN_FUNCTIONS ON CACHE BOOL "")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator)
target_compile_features(VulkanMemoryAllocator PUBLIC cxx_std_23)
