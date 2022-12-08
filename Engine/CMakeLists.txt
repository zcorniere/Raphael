project(RaphaelEngine)

fetchcontent_declare(
    logger
    GIT_REPOSITORY https://github.com/zcorniere/logger-cpp.git
    GIT_TAG v1.0.3
)
fetchcontent_getproperties(logger)
if(NOT logger_POPULATED)
    message(STATUS "Populating logger")
    fetchcontent_populate(logger)
    add_subdirectory(${logger_SOURCE_DIR} ${logger_BINARY_DIR})
    target_compile_definitions(logger PUBLIC LOGGER_EXTERN_DECLARATION)
endif()

add_library(
    ${PROJECT_NAME} STATIC
    # Select the correct platform to compile
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>: Platforms/Unix/UnixPlatform.cxx>
    $<$<CXX_COMPILER_ID:MSVC>: Platforms/Windows/WindowsPlatform.cxx>
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC logger)

target_precompile_headers(${PROJECT_NAME} PRIVATE Raphael.hxx)

target_compile_options(
    ${PROJECT_NAME}
    PUBLIC $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
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
