cmake_minimum_required(VERSION 3.21)
project(GrowAGarden LANGUAGES CXX)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Testing infrastructure ──────────────────────────────────────────────────
enable_testing()
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Download SFML 3 automatically
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        3.0.0 # Or whatever specific 3.x version you want
)
FetchContent_MakeAvailable(SFML)

# Download nlohmann/json (header-only)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

# ── Include directories ──────────────────────────────────────────────────────
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# ── Game source files (globbed subsystems) ───────────────────────────────────
file(GLOB_RECURSE GAME_SOURCES
    "${CMAKE_SOURCE_DIR}/src/entities/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/items/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/world/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/systems/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/ui/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/game/*.cpp"
)
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*main\\.cpp$")

# ── Title screen sources (explicit) ─────────────────────────────────────────
set(TITLE_SOURCES
    src/title/Button.cpp
    src/title/Button.h
    src/title/CreditsScreen.cpp
    src/title/CreditsScreen.h
    src/title/TitleScreen.cpp
    src/title/TitleScreen.h
    src/title/TitleScreenApp.cpp
    src/title/TitleScreenApp.h
)

# ── Main executable ──────────────────────────────────────────────────────────
add_executable(GrowAGarden
    src/main.cpp
    ${TITLE_SOURCES}
    ${GAME_SOURCES}
)
target_link_libraries(GrowAGarden PRIVATE
    SFML::Graphics SFML::Window SFML::System SFML::Audio
    nlohmann_json::nlohmann_json
)

# ── Copy assets ──────────────────────────────────────────────────────────────
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    add_custom_command(TARGET GrowAGarden POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:GrowAGarden>"
        COMMENT "Copying assets to build directory"
    )
endif()

# ── Tests ────────────────────────────────────────────────────────────────────
add_subdirectory(tests)