cmake_minimum_required(VERSION 3.21)
project(GrowAGarden)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML3
find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Source files
set(SOURCES
    src/main.cpp
    src/title/Button.cpp
    src/title/Button.h
    src/title/CreditsScreen.cpp
    src/title/CreditsScreen.h
    src/title/TitleScreen.cpp
    src/title/TitleScreen.h
    src/title/TitleScreenApp.cpp
    src/title/TitleScreenApp.h
)

# Create executable
add_executable(GrowAGarden ${SOURCES})

# Link SFML
target_link_libraries(GrowAGarden PRIVATE SFML::Graphics SFML::Window SFML::System)

# Copy assets if they exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    add_custom_command(TARGET GrowAGarden POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<CONFIG>"
        COMMENT "Copying assets to build directory"
    )
endif()
