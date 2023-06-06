# Unit tests building

enable_testing()

function(build_tests TARGET)
    if(BUILD_TESTING)
        message(STATUS "Building unit tests for ${PROJECT_NAME}")
        set(TEST_NAME ${TARGET}_Test)
        add_executable(${TEST_NAME} ${ARGN})
        target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests/)
        target_link_libraries(${TEST_NAME} PRIVATE Catch2::Catch2WithMain ${TARGET})

        add_dependencies(tests ${TEST_NAME})

        if(DEFINED ENV{CI})
            catch_discover_tests(${TEST_NAME} TEST_SPEC "~[no_ci]")
        else()
            catch_discover_tests(${TEST_NAME})
        endif()
    endif()
endfunction()
