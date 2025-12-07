cmake_minimum_required(VERSION 3.15)
project(EpidemicSim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Copy font into build dir
configure_file(arial.ttf arial.ttf COPYONLY)

# SFML 3
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

add_executable(epidemic
    main.cpp
)

target_link_libraries(epidemic PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)

# Warnings
if (MSVC)
    target_compile_options(epidemic PRIVATE /W4)
else()
    target_compile_options(epidemic PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_custom_target(run
    COMMAND epidemic
    DEPENDS epidemic
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running epidemic simulation..."
)


add_custom_target(timelapse
    COMMAND epidemic
    COMMAND ffmpeg
        -framerate 4
        -i frames/frame_%04d.png
        -vf scale=1310:1050
        -c:v libx264
        -pix_fmt yuv420p
        ../epidemic_timelapse.mp4
    DEPENDS epidemic
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running simulation and creating timelapse video with ffmpeg..."
)
