project(RaphaelEngine)

if(WIN32)
    file(GLOB PLATFORM_SOURCE_FILES Platforms/Windows/*.cxx)
elseif(UNIX)
    file(GLOB PLATFORM_SOURCE_FILES Platforms/Unix/*.cxx)
else()
    message(FATAL_ERROR "Platform Not supported !")
endif()

add_library(
    ${PROJECT_NAME} STATIC
    Threading/Thread.cxx
    Threading/ThreadPool.cxx
    Core/Application.cxx
    Core/Ref.cxx
    ${PLATFORM_SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC logger glm glfw)

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
