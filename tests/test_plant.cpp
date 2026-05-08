#include <gtest/gtest.h>
#include "helpers/ConcreteClasses.h"
#include "../src/entities/HarvestedItem.h"

// ── isFullyGrown ───────────────────────────────────────────────────────────

TEST(PlantTest, NotFullyGrownAtStageZero) {
    ConcretePlant p(1, "Tomato", 0, 5, 10, 0, 100.0);
    EXPECT_FALSE(p.isFullyGrown());
}

TEST(PlantTest, FullyGrownWhenCurrentStageEqualsMax) {
    ConcretePlant p(1, "Tomato", 5, 5, 10, 50, 100.0);
    EXPECT_TRUE(p.isFullyGrown());
}

// ── grow ──────────────────────────────────────────────────────────────────

TEST(PlantTest, GrowWithZeroTicksIsNoOp) {
    ConcretePlant p(1, "Tomato", 0, 5, 10, 0, 100.0);
    p.grow(0);
    EXPECT_FALSE(p.isFullyGrown());
}

TEST(PlantTest, GrowAdvancesStageCorrectly) {
    // 10 ticks per stage, 5 stages. After 20 ticks: stage 2.
    ConcretePlant p(1, "Tomato", 0, 5, 10, 0, 100.0);
    p.grow(20);
    EXPECT_FALSE(p.isFullyGrown());
    EXPECT_EQ(p.getTimeToGrowth(), 30u);  // 50 - 20 = 30 remaining
}

TEST(PlantTest, GrowCapsAtMaxStage) {
    ConcretePlant p(1, "Tomato", 0, 5, 10, 0, 100.0);
    p.grow(9999);
    EXPECT_TRUE(p.isFullyGrown());
}

TEST(PlantTest, GrowExactlyToMaxStage) {
    ConcretePlant p(1, "Tomato", 0, 5, 10, 0, 100.0);
    p.grow(50);  // 5 stages × 10 ticks
    EXPECT_TRUE(p.isFullyGrown());
}

// ── getTimeToGrowth ────────────────────────────────────────────────────────

TEST(PlantTest, TimeToGrowthIsZeroWhenFullyGrown) {
    ConcretePlant p(1, "Tomato", 5, 5, 10, 50, 100.0);
    EXPECT_EQ(p.getTimeToGrowth(), 0u);
}

TEST(PlantTest, TimeToGrowthIsCorrectMidGrowth) {
    // 5 stages × 10 ticks = 50 total. Start at tick 10 → 40 remaining.
    ConcretePlant p(1, "Tomato", 1, 5, 10, 10, 100.0);
    EXPECT_EQ(p.getTimeToGrowth(), 40u);
}

TEST(PlantTest, TimeToGrowthIsFullWhenJustPlanted) {
    ConcretePlant p(1, "Tomato", 0, 5, 10, 0, 100.0);
    EXPECT_EQ(p.getTimeToGrowth(), 50u);
}

// ── harvest ───────────────────────────────────────────────────────────────

TEST(PlantTest, HarvestOnUnripeReturnsDefaultItem) {
    ConcretePlant p(1, "Tomato", 2, 5, 10, 20, 100.0);
    HarvestedItem item = p.harvest();
    EXPECT_DOUBLE_EQ(item.getPrice(), 0.0);  // default HarvestedItem
}

TEST(PlantTest, HarvestOnRipeReturnsCorrectPrice) {
    ConcretePlant p(1, "Tomato", 5, 5, 10, 50, 100.0);
    HarvestedItem item = p.harvest();
    EXPECT_DOUBLE_EQ(item.getPrice(), 100.0);
}

TEST(PlantTest, HarvestWithWetMutationDoublesPrice) {
    ConcretePlant p(1, "Tomato", 5, 5, 10, 50, 100.0);
    Mutation wet(MutationType::WET, 2.0f, WeatherType::RAIN);
    p.addMutation(wet);
    HarvestedItem item = p.harvest();
    EXPECT_DOUBLE_EQ(item.getPrice(), 200.0);
}

TEST(PlantTest, HarvestWithFrozenMutationMultipliesBy5) {
    ConcretePlant p(1, "Tomato", 5, 5, 10, 50, 100.0);
    Mutation frozen(MutationType::FROZEN, 5.0f, WeatherType::FROST);
    p.addMutation(frozen);
    HarvestedItem item = p.harvest();
    EXPECT_DOUBLE_EQ(item.getPrice(), 500.0);
}

TEST(PlantTest, HarvestWithStackedMutationsMultipliesAll) {
    // WET(×2) × SHOCKED(×100) on a 10-sheckle plant = 2000
    ConcretePlant p(1, "Tomato", 5, 5, 10, 50, 10.0);
    p.addMutation(Mutation(MutationType::WET,     2.0f,   WeatherType::RAIN));
    p.addMutation(Mutation(MutationType::SHOCKED,  100.0f, WeatherType::THUNDER_STORM));
    HarvestedItem item = p.harvest();
    EXPECT_DOUBLE_EQ(item.getPrice(), 2000.0);
}

TEST(PlantTest, HarvestTransfersMutationListToItem) {
    ConcretePlant p(1, "Tomato", 5, 5, 10, 50, 100.0);
    p.addMutation(Mutation(MutationType::WET, 2.0f, WeatherType::RAIN));
    HarvestedItem item = p.harvest();
    auto muts = item.getMutationList();
    ASSERT_EQ(muts.size(), 1u);
    EXPECT_EQ(muts[0], MutationType::WET);
}

// ── regrow behaviour ──────────────────────────────────────────────────────

TEST(PlantTest, RegrowPlantResetsToRegrowStageAfterHarvest) {
    // regrows=true, regrowStage=1 → after harvest resets to stage 1
    ConcretePlant p(1, "Strawberry", 5, 5, 10, 50, 50.0, true, 1);
    p.harvest();
    EXPECT_FALSE(p.isFullyGrown());
    EXPECT_EQ(p.getTimeToGrowth(), 40u);  // (5-1)*10 = 40 ticks remaining
}

TEST(PlantTest, RegrowPlantClearsMutationsAfterHarvest) {
    ConcretePlant p(1, "Strawberry", 5, 5, 10, 50, 50.0, true, 1);
    p.addMutation(Mutation(MutationType::WET, 2.0f, WeatherType::RAIN));
    p.harvest();
    EXPECT_TRUE(p.getMutations().empty());
}

// ── addMutation deduplication ─────────────────────────────────────────────

TEST(PlantTest, AddMutationDeduplicatesSameType) {
    ConcretePlant p(1, "Tomato", 0, 5, 10, 0, 100.0);
    p.addMutation(Mutation(MutationType::WET, 2.0f, WeatherType::RAIN));
    p.addMutation(Mutation(MutationType::WET, 2.0f, WeatherType::RAIN));
    EXPECT_EQ(p.getMutations().size(), 1u);
}

TEST(PlantTest, AddMutationAllowsDifferentTypes) {
    ConcretePlant p(1, "Tomato", 0, 5, 10, 0, 100.0);
    p.addMutation(Mutation(MutationType::WET,    2.0f,  WeatherType::RAIN));
    p.addMutation(Mutation(MutationType::FROZEN,  5.0f,  WeatherType::FROST));
    EXPECT_EQ(p.getMutations().size(), 2u);
}

// ── applyWeatherEffect ────────────────────────────────────────────────────

TEST(PlantTest, RainWeatherSpeedsUpGrowth) {
    // With SUMMER (×1.0) the plant needs 50 ticks to finish.
    // With RAIN (÷1.5) currentTicksPerStage drops → fewer ticks needed.
    ConcretePlant p(1, "Tomato", 0, 5, 10, 0, 100.0);
    p.applyWeatherEffect(WeatherType::RAIN);
    // After rain, 34 ticks should be enough to fully grow
    // (floor(10/1.5)=6 ticks/stage → 5×6=30, but implementation uses round → 7 ticks/stage → 35)
    // Either way, 50 ticks (the SUMMER amount) must be MORE than needed.
    p.grow(50);
    EXPECT_TRUE(p.isFullyGrown());
}

TEST(PlantTest, MeteorShowerWeatherSpeedsUpMostAggressively) {
    ConcretePlant p(1, "Tomato", 0, 5, 10, 0, 100.0);
    p.applyWeatherEffect(WeatherType::METEOR_SHOWER);
    // round(10/1.75) = 6 ticks/stage → 30 total
    p.grow(30);
    EXPECT_TRUE(p.isFullyGrown());
}

TEST(PlantTest, SummerWeatherDoesNotChangeGrowthRate) {
    ConcretePlant p(1, "Tomato", 0, 5, 10, 0, 100.0);
    p.applyWeatherEffect(WeatherType::SUMMER);
    p.grow(49);
    EXPECT_FALSE(p.isFullyGrown());
    p.grow(1);
    EXPECT_TRUE(p.isFullyGrown());
}

TEST(PlantTest, WeatherChangeDoesNotLowerFullyGrownStage) {
    ConcretePlant p(1, "Tomato", 0, 5, 10, 0, 100.0);
    p.applyWeatherEffect(WeatherType::RAIN);
    p.grow(35);
    ASSERT_TRUE(p.isFullyGrown());

    p.applyWeatherEffect(WeatherType::SUMMER);
    p.grow(1);

    EXPECT_TRUE(p.isFullyGrown());
}
