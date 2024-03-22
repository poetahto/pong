cmake_minimum_required(VERSION 3.5)
project(pong)

add_subdirectory(${PROJECT_SOURCE_DIR}/lib/raylib)
add_executable(Game src/main.c)
target_link_libraries(Game raylib)
target_include_directories(Game PUBLIC ${PROJECT_SOURCE_DIR}/lib/incbin)

# CREATING BIN DIRECTORY

add_custom_target(GameAssets
        COMMAND ${CMAKE_COMMAND} -E
        copy_directory_if_different ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}
)
add_dependencies(Game GameAssets)

add_custom_command(TARGET Game
        POST_BUILD
        # Remove the old "bin" directory
        COMMAND ${CMAKE_COMMAND} -E
        rm -rf ${PROJECT_SOURCE_DIR}/bin
        # Create a new "bin" directory
        COMMAND ${CMAKE_COMMAND} -E
        make_directory ${PROJECT_SOURCE_DIR}/bin
        # Copy the executable to "bin"
        COMMAND ${CMAKE_COMMAND} -E
        copy $<TARGET_FILE:Game> ${PROJECT_SOURCE_DIR}/bin/pong.exe
)

