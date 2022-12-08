project(RaphaelEngine)

add_library(
    ${PROJECT_NAME} STATIC
    Threading/Thread.cxx
    Threading/ThreadPool.cxx
    # Select the correct platform to compile
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
    Platforms/Unix/UnixPlatform.cxx>
    $<$<CXX_COMPILER_ID:MSVC>:
    Platforms/Windows/WindowsPlatform.cxx>
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC logger glm)

target_precompile_headers(${PROJECT_NAME} PRIVATE Raphael.hxx)

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
