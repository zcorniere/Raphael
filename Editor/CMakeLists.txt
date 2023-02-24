project(RaphaelEditor)

option(NO_PROFILING "Disable the profiling" ON)

add_executable(${PROJECT_NAME} main.cxx)

target_link_libraries(${PROJECT_NAME} PUBLIC RaphaelEngine)
target_precompile_headers(${PROJECT_NAME} REUSE_FROM RaphaelEngine)
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
            /wd4324>
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
set_target_properties(${PROJECT_NAME} PROPERTIES ENABLE_EXPORTS 1)

if(NOT NO_PROFILING)
    message(STATUS "Profiling is disabled")
    target_compile_definitions(${PROJECT_NAME} PUBLIC TRACY_ENABLE=1)
    target_link_libraries(${PROJECT_NAME} PUBLIC Tracy::TracyClient)
endif(NOT NO_PROFILING)

# target_generate_debug_file(${PROJECT_NAME})
