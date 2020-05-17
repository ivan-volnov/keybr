cmake_minimum_required(VERSION 3.5)

project(keybr LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

add_subdirectory(libs/sqlite_database)
add_subdirectory(libs/string_essentials)
add_subdirectory(libs/tiled_ncurses)

include_directories(./)

add_executable(${PROJECT_NAME}
    src/app.cpp
    src/app.h
    src/config.cpp
    src/config.h
    src/global.h
    src/main.cpp
    src/phrase.cpp
    src/phrase.h
    src/trainer.cpp
    src/trainer.h
    src/trainer_data.cpp
    src/trainer_data.h
    src/utility/anki_client.cpp
    src/utility/anki_client.h
    src/utility/average.h
    src/utility/curl_request.cpp
    src/utility/curl_request.h
    src/utility/speech_engine.cpp
    src/utility/speech_engine.h
    src/utility/tools.cpp
    src/utility/tools.h
)

target_link_libraries(${PROJECT_NAME}
    string_essentials
    tiled_ncurses
    sqlite_database
    curl
    "-framework ApplicationServices"
)

install (TARGETS
    ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
