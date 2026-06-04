cmake_minimum_required(VERSION 3.16)
project(GameConsole114_2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

# 已下載過就不要每次 configure 再連 Git（避免「Re-running CMake」卡住）
set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "Skip FetchContent updates" FORCE)

include(FetchContent)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
if(NOT raylib_POPULATED)
    message(STATUS "Fetching raylib 5.5 (tarball, not git) — first time may take a few minutes...")
endif()
FetchContent_MakeAvailable(raylib)

add_subdirectory(chess_war)
add_subdirectory(bomberman)
add_subdirectory(hub)
