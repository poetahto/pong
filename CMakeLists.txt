cmake_minimum_required(VERSION 3.5)
project(pong)

# SETTINGS

set(GAME_TARGET pong)

if(WIN32)
    set(CMAKE_WIN32_EXECUTABLE TRUE)
endif()

# GAME TARGET

add_executable(${GAME_TARGET} src/main.c)

if(WIN32)
    target_link_libraries(${GAME_TARGET} user32.lib)
endif()

# CREATING BIN DIRECTORY

add_custom_command(TARGET ${GAME_TARGET} POST_BUILD 
    # Remove the old "bin" directory
    COMMAND ${CMAKE_COMMAND} -E 
    rm -rf ${PROJECT_SOURCE_DIR}/bin

    # Create a new "bin" directory
    COMMAND ${CMAKE_COMMAND} -E 
    make_directory ${PROJECT_SOURCE_DIR}/bin

    # Copy the executable to "bin"
    COMMAND ${CMAKE_COMMAND} -E 
    copy $<TARGET_FILE:${GAME_TARGET}> ${PROJECT_SOURCE_DIR}/bin

    # Copy the assets folder to "bin"
    COMMAND ${CMAKE_COMMAND} -E 
    copy_directory ${PROJECT_SOURCE_DIR}/assets ${PROJECT_SOURCE_DIR}/bin/assets
)

