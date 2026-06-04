cmake_minimum_required(VERSION 3.16)

set(SOURCES
    src/Game.cpp
    src/GameUI.cpp
    src/HudLayout.cpp
    src/AssetManager.cpp
    src/Entity.cpp
    src/Hero.cpp
    src/Minion.cpp
    src/Board.cpp
    src/TurnSystem.cpp
    src/MovementValidator.cpp
)

add_library(chess_war_core STATIC ${SOURCES})
target_include_directories(chess_war_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/hub/include
)
target_link_libraries(chess_war_core PUBLIC raylib)

add_executable(chess_war src/main.cpp)
target_link_libraries(chess_war PRIVATE chess_war_core)

add_custom_command(TARGET chess_war POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:chess_war>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/bomberman/assets"
            "$<TARGET_FILE_DIR:chess_war>/assets"
)
