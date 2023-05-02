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
