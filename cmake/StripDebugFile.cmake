function(target_generate_debug_file TARGET_NAME)
    add_custom_command(
        TARGET ${TARGET_NAME}
        POST_BUILD
        COMMENT "Generating ${TARGET_NAME} debug file"
        COMMAND bash ${CMAKE_SOURCE_DIR}/cmake/strip_debug_file.sh $<TARGET_FILE:${TARGET_NAME}>
                $<TARGET_FILE:${TARGET_NAME}>.sym
                DEPENDS ${TARGET_NAME}
    )
    set_source_files_properties( $<TARGET_FILE:${TARGET_NAME}>.sym PROPERTIES GENERATED TRUE)
endfunction(target_generate_debug_file)
