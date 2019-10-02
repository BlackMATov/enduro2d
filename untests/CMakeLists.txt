function(add_e2d_tests NAME)
    set(TESTS_NAME untests_${NAME})

    #
    # sources
    #

    file(GLOB ${TESTS_NAME}_sources
        sources/*.*
        sources/${TESTS_NAME}/*.*)
    set(TESTS_SOURCES ${${TESTS_NAME}_sources})
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TESTS_SOURCES})

    #
    # executable
    #

    add_executable(${TESTS_NAME} ${TESTS_SOURCES})
    target_link_libraries(${TESTS_NAME} enduro2d)
    set_target_properties(${TESTS_NAME} PROPERTIES FOLDER untests)

    target_compile_options(${TESTS_NAME}
        PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:
            /W3 /MP /bigobj>
        PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
            -Wall -Wextra -Wpedantic>)

    #
    # resources
    #

    add_custom_command(TARGET ${TESTS_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/bin
        $<TARGET_FILE_DIR:${TESTS_NAME}>/bin)
    add_e2d_shared_libraries_to_target(${TESTS_NAME})

    #
    # tests
    #

    add_test(${TESTS_NAME} ${TESTS_NAME})
endfunction(add_e2d_tests)

add_e2d_tests(base)
add_e2d_tests(core)
add_e2d_tests(high)
add_e2d_tests(math)
add_e2d_tests(utils)
