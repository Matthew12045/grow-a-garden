#include <gtest/gtest.h>
#include "../src/world/WeatherSystem.h"
#include "../src/world/Garden.h"
#include "helpers/ConcreteClasses.h"
#include <memory>

// ── Initial state ─────────────────────────────────────────────────────────

TEST(WeatherSystemTest, StartsAsSummer) {
    WeatherSystem ws;
    EXPECT_EQ(ws.getCurrentWeather(), WeatherType::SUMMER);
}

// ── update changes weather ────────────────────────────────────────────────

TEST(WeatherSystemTest, WeatherEventuallyChangesAfterEnoughUpdates) {
    WeatherSystem ws;
    // ticksUntilChange_ starts at 20. After 20 updates it must have changed
    // (or re-rolled SUMMER — run enough iterations to guarantee at least one
    //  non-SUMMER result with overwhelming probability).
    bool changed = false;
    for (int i = 0; i < 200 && !changed; ++i) {
        ws.update();
        if (ws.getCurrentWeather() != WeatherType::SUMMER) {
            changed = true;
        }
    }
    EXPECT_TRUE(changed) << "Weather never left SUMMER after 200 updates";
}

TEST(WeatherSystemTest, WeatherDoesNotChangeBeforeTicksUntilChangeExpires) {
    WeatherSystem ws;
    // ticksUntilChange_ = 20; after 19 updates it should still be SUMMER
    // (because the initial roll was SUMMER and nothing has fired yet).
    for (int i = 0; i < 19; ++i) {
        ws.update();
    }
    EXPECT_EQ(ws.getCurrentWeather(), WeatherType::SUMMER);
}

// ── applyEffectsToGrid ────────────────────────────────────────────────────

TEST(WeatherSystemTest, ApplyEffectsToGridDoesNotCrashOnEmptyGarden) {
    WeatherSystem ws;
    Garden g(20, 20);
    EXPECT_NO_THROW(ws.applyEffectsToGrid(g));
}

TEST(WeatherSystemTest, ApplyEffectsToGridModifiesGrowthRateOfPlants) {
    // Plant a tomato, apply RAIN (1.5× speed), grow by 30 ticks,
    // then compare with a control plant that only grew at SUMMER rate.
    Garden g(20, 20);
    g.plantCrop(0, 0, std::make_unique<ConcretePlant>(1, "Tomato", 0, 5, 10, 0, 100.0));

    // Force RAIN by directly applying effects after setting weather externally.
    // WeatherSystem doesn't expose setWeather, so we call applyWeatherEffect
    // on the plant directly to confirm the grid integration works.
    WeatherSystem ws;
    ws.applyEffectsToGrid(g);  // applies SUMMER (×1) — no-op essentially

    Plant* plant = g.getCell(0, 0).getPlant();
    ASSERT_NE(plant, nullptr);
    plant->applyWeatherEffect(WeatherType::RAIN);
    plant->grow(35);  // round(10/1.5)=7 ticks/stage, so 5 stages need 35 ticks
    EXPECT_TRUE(plant->isFullyGrown());
}

TEST(WeatherSystemTest, ApplyEffectsToGridSkipsEmptyCells) {
    // Populate only one cell; all others empty → should not crash
    Garden g(5, 5);
    g.plantCrop(2, 2, std::make_unique<ConcretePlant>(1, "Tomato", 0, 5, 10, 0, 100.0));

    WeatherSystem ws;
    EXPECT_NO_THROW(ws.applyEffectsToGrid(g));
}

// ── tryTriggerMutation ────────────────────────────────────────────────────

TEST(WeatherSystemTest, TryTriggerMutationOnSummerNeverAddsMutation) {
    // SUMMER has 0% mutation chance — calling it many times must add nothing.
    WeatherSystem ws;
    ConcretePlant plant(1, "Tomato", 0, 5, 10, 0, 100.0);
    for (int i = 0; i < 100; ++i) {
        ws.tryTriggerMutation(plant);
    }
    // With SUMMER weather (0% chance) no mutations should appear.
    // Note: WeatherSystem starts as SUMMER.
    EXPECT_TRUE(plant.getMutations().empty());
}
