# logger
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/logger-cpp)
target_compile_definitions(logger PUBLIC LOGGER_EXTERN_DECLARATION)

# glm
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glm)
target_compile_definitions(glm INTERFACE GLM_FORCE_DEPTH_ZERO_TO_ONE)
