cmake_minimum_required(VERSION 3.5)
project(pong)

# LIBRARIES

add_subdirectory(${PROJECT_SOURCE_DIR}/lib/raylib)

# GAME

set(GAME_NAME pong)
set(GAME_SOURCES
    ${PROJECT_SOURCE_DIR}/src/main.c
    ${PROJECT_SOURCE_DIR}/src/player.c
    ${PROJECT_SOURCE_DIR}/src/ball.c
    ${PROJECT_SOURCE_DIR}/src/objective.c
    ${PROJECT_SOURCE_DIR}/src/sound.c
    ${PROJECT_SOURCE_DIR}/src/game.c
)

add_executable(Game ${GAME_SOURCES} data.c)
target_link_libraries(Game raylib)
target_include_directories(Game PUBLIC ${PROJECT_SOURCE_DIR}/lib/incbin ${PROJECT_SOURCE_DIR}/include)

# COPYING GAME ASSETS

add_custom_target(GameAssets
        COMMAND ${CMAKE_COMMAND} -E
        copy_directory_if_different ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}

        # Todo: this isnt crossplatform, prolly compile it properly... look into that
        COMMAND ${PROJECT_SOURCE_DIR}/lib/incbin/incbin_tool.exe ${GAME_SOURCES}
)
add_dependencies(Game GameAssets)

# CREATING BIN DIRECTORY

add_custom_command(TARGET Game POST_BUILD
        # Remove the old "bin" directory
        COMMAND ${CMAKE_COMMAND} -E
        rm -rf ${PROJECT_SOURCE_DIR}/bin

        # Create a new "bin" directory
        COMMAND ${CMAKE_COMMAND} -E
        make_directory ${PROJECT_SOURCE_DIR}/bin

        # Copy the executable to "bin" - todo, some platforms dont want an "exe" extension
        COMMAND ${CMAKE_COMMAND} -E
        copy $<TARGET_FILE:Game> ${PROJECT_SOURCE_DIR}/bin/${GAME_NAME}${CMAKE_EXECUTABLE_SUFFIX}
)

