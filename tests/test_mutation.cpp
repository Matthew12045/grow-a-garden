#include <gtest/gtest.h>
#include "../src/entities/Mutation.h"
#include "helpers/ConcreteClasses.h"

// ── Mutation: construction & getters ───────────────────────────────────────

TEST(MutationTest, StoresTypeCorrectly) {
    Mutation m(MutationType::WET, 2.0f, WeatherType::RAIN);
    EXPECT_EQ(m.getType(), MutationType::WET);
}

TEST(MutationTest, StoresMultiplierCorrectly) {
    Mutation m(MutationType::FROZEN, 5.0f, WeatherType::FROST);
    EXPECT_FLOAT_EQ(m.getMultiplier(), 5.0f);
}

TEST(MutationTest, StoresTriggerWeatherCorrectly) {
    Mutation m(MutationType::SHOCKED, 100.0f, WeatherType::THUNDER_STORM);
    EXPECT_EQ(m.getTrigger(), WeatherType::THUNDER_STORM);
}

TEST(MutationTest, CelestialMultiplierIs150) {
    Mutation m(MutationType::CELESTIAL, 150.0f, WeatherType::METEOR_SHOWER);
    EXPECT_FLOAT_EQ(m.getMultiplier(), 150.0f);
}

// ── Mutation::apply ────────────────────────────────────────────────────────

TEST(MutationTest, ApplyAddsMutationToPlant) {
    ConcretePlant plant(1, "Carrot", 0, 5, 10, 0, 50.0);
    Mutation m(MutationType::WET, 2.0f, WeatherType::RAIN);
    m.apply(plant);

    auto mutations = plant.getMutations();
    ASSERT_EQ(mutations.size(), 1u);
    EXPECT_EQ(mutations[0], MutationType::WET);
}

TEST(MutationTest, ApplyDoesNotDuplicateSameType) {
    ConcretePlant plant(1, "Carrot", 0, 5, 10, 0, 50.0);
    Mutation m(MutationType::WET, 2.0f, WeatherType::RAIN);
    m.apply(plant);
    m.apply(plant);  // second apply should be a no-op

    EXPECT_EQ(plant.getMutations().size(), 1u);
}
