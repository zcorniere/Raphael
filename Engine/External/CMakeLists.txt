set(CMAKE_FOLDER "Raphael/Engine/External")

# logger
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/logger-cpp EXCLUDE_FROM_ALL)

# magic_enum
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/magic_enum EXCLUDE_FROM_ALL)

# xdgpp
add_library(xdgpp INTERFACE EXCLUDE_FROM_ALL)
target_include_directories(xdgpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/xdgpp)
target_compile_features(xdgpp INTERFACE cxx_std_17)

# mimalloc
set(MI_OVERRIDE
    OFF
    CACHE BOOL "" FORCE
)
set(MI_BUILD_TESTS
    OFF
    CACHE INTERNAL "" FORCE
)
set(MI_BUILD_OBJECT
    OFF
    CACHE INTERNAL "" FORCE
)
set(MI_BUILD_SHARED
    OFF
    CACHE INTERNAL "" FORCE
)
set(MI_TRACK_VALGRIND
    $<IF:$<CONFIG:Debug>,ON,OFF>
    CACHE INTERNAL "" FORCE
)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mimalloc)

# glfw
set(GLFW_BUILD_EXAMPLES
    OFF
    CACHE INTERNAL "" FORCE
)
set(GLFW_BUILD_TESTS
    OFF
    CACHE INTERNAL "" FORCE
)
set(GLFW_BUILD_DOCS
    OFF
    CACHE INTERNAL "" FORCE
)
set(GLFW_INSTALL
    OFF
    CACHE INTERNAL "" FORCE
)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glfw EXCLUDE_FROM_ALL)
if(SELECTED_RHI STREQUAL "Vulkan")
    target_link_libraries(glfw INTERFACE Vulkan::Headers)
    target_compile_definitions(glfw INTERFACE GLFW_INCLUDE_NONE)
endif()

# Modern Dialogs
add_library(
    ModernDialogs EXCLUDE_FROM_ALL STATIC ${CMAKE_CURRENT_SOURCE_DIR}/ModernDialogs/ModernDialogs/ModernDialogs.cpp
)
target_include_directories(ModernDialogs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ModernDialogs/ModernDialogs)

# glm
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glm EXCLUDE_FROM_ALL)

