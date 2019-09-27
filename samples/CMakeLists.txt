function(add_e2d_sample NAME)
    set(SAMPLE_NAME sample_${NAME})

    #
    # sources
    #

    file(GLOB ${SAMPLE_NAME}_sources
        sources/*.*
        sources/${SAMPLE_NAME}/*.*)
    set(SAMPLE_SOURCES ${${SAMPLE_NAME}_sources})
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SAMPLE_SOURCES})

    #
    # executable
    #

    add_executable(${SAMPLE_NAME} ${SAMPLE_SOURCES})
    target_link_libraries(${SAMPLE_NAME} enduro2d)
    set_target_properties(${SAMPLE_NAME} PROPERTIES FOLDER samples)

    #
    # resources
    #

    add_custom_command(TARGET ${SAMPLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/bin
        $<TARGET_FILE_DIR:${SAMPLE_NAME}>/bin)
    add_e2d_shared_libraries_to_target(${SAMPLE_NAME})
endfunction(add_e2d_sample)

add_e2d_sample(00)
add_e2d_sample(01)
add_e2d_sample(02)
add_e2d_sample(03)
add_e2d_sample(04)
add_e2d_sample(05)
add_e2d_sample(06)
add_e2d_sample(07)
add_e2d_sample(08)
add_e2d_sample(09)
add_e2d_sample(10)

