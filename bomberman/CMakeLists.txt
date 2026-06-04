cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BOMBERMAN_CORE_SOURCES
    src/GameConfig.cpp
    src/Grid.cpp
    src/Bomb.cpp
    src/Explosion.cpp
    src/Player.cpp
    src/Pathfinder.cpp
    src/AI_Enemy.cpp
    src/EnemyRLPolicy.cpp
    src/Game.cpp
    src/SaveData.cpp
    src/NetSession.cpp
)

add_library(bomberman_core ${BOMBERMAN_CORE_SOURCES})
target_include_directories(bomberman_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
if(WIN32)
    target_link_libraries(bomberman_core PUBLIC ws2_32)
endif()

set(BOMBERMAN_TERMINAL_SOURCES
    src/Input.cpp
    src/Renderer.cpp
    src/TerminalApp.cpp
)

add_library(bomberman_terminal ${BOMBERMAN_TERMINAL_SOURCES})
target_link_libraries(bomberman_terminal PUBLIC bomberman_core)
if(WIN32)
    target_link_libraries(bomberman_terminal PUBLIC kernel32)
endif()

add_executable(bomberman src/main.cpp)
target_link_libraries(bomberman PRIVATE bomberman_terminal)

option(BUILD_GUI "Build Raylib 2D window version (bomberman_gui)" ON)

if(BUILD_GUI)
    if(NOT TARGET raylib)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
        include(FetchContent)
        FetchContent_Declare(
            raylib
            GIT_REPOSITORY https://github.com/raysan5/raylib.git
            GIT_TAG 5.5
        )
        FetchContent_MakeAvailable(raylib)
    endif()

    add_library(bomberman_gfx STATIC src/GfxApp.cpp src/GfxAssets.cpp)
    target_compile_definitions(bomberman_gfx PRIVATE ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")
    target_include_directories(bomberman_gfx PUBLIC ${CMAKE_SOURCE_DIR}/hub/include)
    target_link_libraries(bomberman_gfx PUBLIC bomberman_core raylib)
    if(MSVC)
        target_compile_options(bomberman_gfx PRIVATE /utf-8)
    else()
        target_compile_options(bomberman_gfx PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
    endif()
    if(WIN32)
        target_link_libraries(bomberman_gfx PUBLIC winmm gdi32 opengl32)
    endif()

    add_executable(bomberman_gui src/main_gui.cpp)
    target_link_libraries(bomberman_gui PRIVATE bomberman_gfx)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts")
    if(EXISTS "C:/Windows/Fonts/kaiu.ttf")
        add_custom_command(
            TARGET bomberman_gui
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "C:/Windows/Fonts/kaiu.ttf"
                    "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/kaiu.ttf"
            COMMENT "Bundle kaiu.ttf (UI fallback if Taipei Sans missing)"
        )
    endif()
    add_custom_command(
        TARGET bomberman_gui
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/assets"
                "$<TARGET_FILE_DIR:bomberman_gui>/assets"
        COMMENT "Copy assets next to bomberman_gui.exe"
    )
endif()

option(BUILD_TESTING "Build Google Test + RapidCheck tests" OFF)

if(BUILD_TESTING)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    FetchContent_Declare(
        rapidcheck
        GIT_REPOSITORY https://github.com/emil-e/rapidcheck.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(rapidcheck)

    add_executable(
        bomberman_tests
        tests/GridTest.cpp
        tests/BombTest.cpp
        tests/ExplosionTest.cpp
        tests/PlayerTest.cpp
        tests/PathfinderTest.cpp
        tests/AIEnemyTest.cpp
        tests/EnemyRLPolicyTest.cpp
        tests/SaveDataTest.cpp
        tests/GameTest.cpp
        tests/RendererTest.cpp
    )

    target_include_directories(
        bomberman_tests
        PRIVATE ${rapidcheck_SOURCE_DIR}/extras/gtest/include
    )

    target_link_libraries(
        bomberman_tests
        PRIVATE bomberman_core GTest::gtest_main rapidcheck
    )

    add_custom_command(
        TARGET bomberman_tests
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/assets"
                "$<TARGET_FILE_DIR:bomberman_tests>/assets"
        COMMENT "Copy assets for tests (RL policy)"
    )

    include(GoogleTest)
    gtest_discover_tests(bomberman_tests)
endif()
