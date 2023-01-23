project(RaphaelEditor)

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
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
set_target_properties(${PROJECT_NAME} PROPERTIES ENABLE_EXPORTS 1)
# target_generate_debug_file(${PROJECT_NAME})
