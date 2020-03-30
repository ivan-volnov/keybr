cmake_minimum_required(VERSION 3.5)

project(keybr LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

add_subdirectory(3rdparty/sqlite3)
include_directories(./)

file(GLOB_RECURSE USER_SOURCES
    src/*.cpp
    src/*.h
)

add_executable(${PROJECT_NAME}
    ${USER_SOURCES}
)

target_link_libraries(${PROJECT_NAME}
    ncurses
    sqlite3_amalgamation
    "-framework ApplicationServices"
)
