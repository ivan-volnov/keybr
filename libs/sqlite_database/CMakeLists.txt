cmake_minimum_required(VERSION 3.5)

project(sqlite_database LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

add_subdirectory(libs/sqlite3)
include_directories(./)
include_directories(include)

add_library(${PROJECT_NAME} STATIC
    include/sqlite_database/sqlite_database.h
    src/sqlite_database.cpp
)

target_link_libraries(${PROJECT_NAME}
    sqlite3_amalgamation
)
