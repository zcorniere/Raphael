set(CMAKE_FOLDER "Raphael/External")

# tracy
set(TRACY_ENABLE
    ON
    CACHE BOOL ""
)
set(TRACY_ON_DEMAND
    ON
    CACHE BOOL ""
)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tracy EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tracy/profiler EXCLUDE_FROM_ALL)

add_library(
    imgui STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.cpp
)
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends)
target_link_libraries(imgui PUBLIC glfw)

if(BUILD_TESTING)

    # Catch2
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Catch2 EXCLUDE_FROM_ALL)

    include(CTest)
    include(Catch)

    set(CMAKE_FOLDER "")
    add_custom_target(tests)
endif(BUILD_TESTING)
