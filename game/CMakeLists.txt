
set(GAME_NAME game)

#
# sources
#

file(GLOB ${GAME_NAME}_sources
    sources/*.*)
set(GAME_SOURCES ${${GAME_NAME}_sources})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${GAME_SOURCES})

#
# executable
#

add_executable(${GAME_NAME} ${GAME_SOURCES})
target_link_libraries(${GAME_NAME} enduro2d)
set_target_properties(${GAME_NAME} PROPERTIES FOLDER games)

#
# resources
#

add_custom_command(TARGET ${GAME_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/bin
    $<TARGET_FILE_DIR:${GAME_NAME}>/bin
    COMMENT "copy resources")
add_e2d_shared_libraries_to_target(${GAME_NAME})

