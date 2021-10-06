file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.cpp)
add_library(${PLUGIN_NAME} SHARED ${SOURCES})

add_custom_command(
        TARGET ${PLUGIN_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.so
        ${PROJECT_DIR}/docker/plugins/${PLUGIN_NAME}.so)