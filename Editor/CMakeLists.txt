project(RaphaelEditor)
set(CMAKE_FOLDER  "Raphael/Editor")


option(RPH_ENABLE_PROFILING "Enable the profiling" OFF)

add_executable(${PROJECT_NAME} main.cxx EditorApplication.cxx)

target_link_libraries(${PROJECT_NAME} PUBLIC RaphaelEngine)
target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/Engine/src/Engine/Raphael.hxx)
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

if (MSVC)
	set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()

if(RPH_ENABLE_PROFILING)
    message(STATUS "Profiling is enabled")
    target_compile_definitions(${PROJECT_NAME} PUBLIC RPH_ENABLE_PROFILING=1)
    target_link_libraries(${PROJECT_NAME} PUBLIC Tracy::TracyClient)
else()
    message(STATUS "Profiling is disabled")
    target_compile_definitions(${PROJECT_NAME} PUBLIC RPH_ENABLE_PROFILING=0)
endif(RPH_ENABLE_PROFILING)

if(SELECTED_RHI STREQUAL "Vulkan")
    target_link_libraries(${PROJECT_NAME} PRIVATE RaphaelRHIVulkan)
    target_compile_definitions(${PROJECT_NAME} PRIVATE RAPHAEL_SELECTED_RHI_VULKAN=1)
else()
    message(FATAL_ERROR "${SELECTED_RHI} is not supported !")
endif()

# target_generate_debug_file(${PROJECT_NAME})
