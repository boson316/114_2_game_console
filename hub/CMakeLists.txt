add_executable(game_console_hub
    src/main.cpp
    src/HubFont.cpp
    src/chess_runner.cpp
    src/bomber_runner.cpp
)

target_include_directories(game_console_hub PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/hub/include
    ${CMAKE_SOURCE_DIR}/chess_war/src
    ${CMAKE_SOURCE_DIR}/bomberman/include
)

target_compile_definitions(game_console_hub PRIVATE
    HUB_ASSETS_DIR="${CMAKE_SOURCE_DIR}/bomberman/assets"
)

target_link_libraries(game_console_hub PRIVATE chess_war_core bomberman_gfx bomberman_core raylib)

if(MSVC)
    target_compile_options(game_console_hub PRIVATE /utf-8)
else()
    target_compile_options(game_console_hub PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

if(WIN32)
    target_link_libraries(game_console_hub PRIVATE winmm gdi32 opengl32 ws2_32)
endif()

add_custom_command(TARGET game_console_hub POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/chess_war/assets"
            "$<TARGET_FILE_DIR:game_console_hub>/chess_assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/bomberman/assets"
            "$<TARGET_FILE_DIR:game_console_hub>/assets"
    COMMENT "Copy chess + bomberman assets next to hub exe"
)
