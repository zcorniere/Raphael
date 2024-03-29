# Find the best linker available (skip on window)
function(find_linker)
    if(MSVC)
        return()
    endif()

    message(STATUS "Looking for mold")
    find_program(MOLD_FOUND mold)
    if(MOLD_FOUND)
        message(STATUS "Looking for mold - found")
    else()
        message(STATUS "Looking for mold - not found")
    endif()
    message(STATUS "Looking for lld")
    find_program(LLD_FOUND lld)
    if(LLD_FOUND)
        message(STATUS "Looking for lld - found")
    else()
        message(STATUS "Looking for lld - not found")
    endif()

    if(MOLD_FOUND)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12.1.0)
            if(LLD_FOUND)
                set(SELECTED_LINKER "lld")
            endif()
        else()
            set(SELECTED_LINKER "mold")
        endif()
    elseif(LLD_FOUND)
        set(SELECTED_LINKER "lld")
    endif()

    if(DEFINED SELECTED_LINKER)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=${SELECTED_LINKER}" PARENT_SCOPE)
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=${SELECTED_LINKER}" PARENT_SCOPE)
        message(STATUS "Linker used - ${SELECTED_LINKER}")
    endif()


endfunction(find_linker)
