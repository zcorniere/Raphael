function(target_disable_rtti target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /GR-)
    elseif(GNU)
        target_compile_options(${target} PRIVATE -fno-rtti)
    elseif(CLANG)
        target_compile_options(${target} PRIVATE -fno-rtti)
    endif()
endfunction(target_disable_rtti)
