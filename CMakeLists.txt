cmake_minimum_required(VERSION 3.26)
project(FOP-project)

set(CMAKE_CXX_STANDARD 17)

add_executable(FOP-project
        main.cpp
        Block.cpp
        Block.h
        FileHandler.cpp
        FileHandler.h
        Sprite.cpp
        Sprite.h
        Sprite.h
        Sound.h
        Sound.cpp
)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

target_link_libraries(${PROJECT_NAME} -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer)
