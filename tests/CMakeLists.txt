# ── Unit tests ─────────────────────────────────────────────────────────────
set(UNIT_TEST_SOURCES
    test_mutation.cpp
    test_plant.cpp
    test_items.cpp
    test_cell_garden.cpp
    test_weather.cpp
    test_inventory_shop.cpp
    test_events.cpp
)

add_executable(GrowAGardenTests ${UNIT_TEST_SOURCES} ${GAME_SOURCES})

# Tell the tests where to find the main game headers (src/) 
# and the local test headers (tests/)
target_include_directories(GrowAGardenTests PRIVATE 
    "${CMAKE_SOURCE_DIR}/src" 
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

# Link Google Test and SFML
target_link_libraries(GrowAGardenTests PRIVATE GTest::gtest GTest::gtest_main SFML::Graphics SFML::Window SFML::Audio SFML::System)

include(GoogleTest)
gtest_discover_tests(GrowAGardenTests)

# ── Integration tests ──────────────────────────────────────────────────────
# (Assuming test_integration.cpp is also in your tests/ folder)
add_executable(GrowAGardenIntegrationTests test_integration.cpp ${GAME_SOURCES})

target_include_directories(GrowAGardenIntegrationTests PRIVATE 
    "${CMAKE_SOURCE_DIR}/src" 
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(GrowAGardenIntegrationTests PRIVATE GTest::gtest GTest::gtest_main SFML::Graphics SFML::Window SFML::Audio SFML::System)
gtest_discover_tests(GrowAGardenIntegrationTests)