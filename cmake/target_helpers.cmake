function(cave_copy_runtime_dll target dll_target)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${dll_target}>
            $<TARGET_FILE_DIR:${target}>
    )
endfunction()
