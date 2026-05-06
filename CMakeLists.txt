cmake_minimum_required(VERSION 3.16)
project(GrowAGarden LANGUAGES CXX)

enable_testing()

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ── Collect all project source files (excluding main.cpp) ──────────────────
file(GLOB_RECURSE GAME_SOURCES
    "${CMAKE_SOURCE_DIR}/src/entities/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/items/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/world/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/systems/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/ui/*.cpp"
)
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*main\\.cpp$")

# # ── Unit tests ─────────────────────────────────────────────────────────────
# set(UNIT_TEST_SOURCES
#     test_mutation.cpp
#     test_plant.cpp
#     test_items.cpp
#     test_cell_garden.cpp
#     test_weather.cpp
#     test_inventory_shop.cpp
#     test_events.cpp
# )

# add_executable(GrowAGardenTests ${UNIT_TEST_SOURCES} ${GAME_SOURCES})
# target_include_directories(GrowAGardenTests PRIVATE "${CMAKE_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}")
# target_link_libraries(GrowAGardenTests PRIVATE GTest::gtest GTest::gtest_main sfml-graphics sfml-window sfml-audio sfml-system)
# include(GoogleTest)
# gtest_discover_tests(GrowAGardenTests)

# # ── Integration tests ──────────────────────────────────────────────────────
# add_executable(GrowAGardenIntegrationTests test_integration.cpp ${GAME_SOURCES})
# target_include_directories(GrowAGardenIntegrationTests PRIVATE "${CMAKE_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}")
# target_link_libraries(GrowAGardenIntegrationTests PRIVATE GTest::gtest GTest::gtest_main sfml-graphics sfml-window sfml-audio sfml-system)
# gtest_discover_tests(GrowAGardenIntegrationTests)

add_subdirectory(tests)