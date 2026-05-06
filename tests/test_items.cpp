#include <gtest/gtest.h>
#include "../src/entities/HarvestedItem.h"
#include "../src/items/Seed.h"
#include "helpers/ConcreteClasses.h"

// ── HarvestedItem ─────────────────────────────────────────────────────────

TEST(HarvestedItemTest, DefaultConstructorHasPriceZero) {
    HarvestedItem item;
    EXPECT_DOUBLE_EQ(item.getPrice(), 0.0);
}

TEST(HarvestedItemTest, GetPriceReturnsFinalSellPrice) {
    HarvestedItem item(250.0, {});
    EXPECT_DOUBLE_EQ(item.getPrice(), 250.0);
}

TEST(HarvestedItemTest, GetMutationListIsEmptyByDefault) {
    HarvestedItem item(100.0, {});
    EXPECT_TRUE(item.getMutationList().empty());
}

TEST(HarvestedItemTest, GetMutationListReturnsAllMutations) {
    std::vector<MutationType> muts = {MutationType::WET, MutationType::SHOCKED};
    HarvestedItem item(100.0, muts);
    ASSERT_EQ(item.getMutationList().size(), 2u);
    EXPECT_EQ(item.getMutationList()[0], MutationType::WET);
    EXPECT_EQ(item.getMutationList()[1], MutationType::SHOCKED);
}

TEST(HarvestedItemTest, GetMutationsStringIsNonEmptyWhenMutationsPresent) {
    // We only verify it doesn't crash and returns something;
    // exact format is an implementation detail.
    std::vector<MutationType> muts = {MutationType::FROZEN};
    HarvestedItem item(50.0, muts);
    EXPECT_FALSE(item.getMutations().empty());
}

// ── Seed ─────────────────────────────────────────────────────────────────

TEST(SeedTest, GetPriceReturnsBasePrice) {
    Seed s(1, "Carrot Seed", "Grows a carrot", 5.0, "Carrot");
    EXPECT_DOUBLE_EQ(s.getPrice(), 5.0);
}

TEST(SeedTest, GetCropTypeReturnsCorrectType) {
    Seed s(1, "Carrot Seed", "Grows a carrot", 5.0, "Carrot");
    EXPECT_EQ(s.getCropType(), "Carrot");
}

TEST(SeedTest, GetNameReturnsCorrectName) {
    Seed s(2, "Strawberry Seed", "Grows a strawberry", 10.0, "Strawberry");
    EXPECT_EQ(s.getName(), "Strawberry Seed");
}

TEST(SeedTest, GetDescrReturnsCorrectDescription) {
    Seed s(3, "Potato Seed", "Grows a potato", 3.0, "Potato");
    EXPECT_EQ(s.getDescr(), "Grows a potato");
}

TEST(SeedTest, IdIsStoredCorrectly) {
    Seed s(42, "Tomato Seed", "desc", 8.0, "Tomato");
    EXPECT_EQ(s.id(), static_cast<std::size_t>(42));
}

// ── Tool (via ConcreteTool) ───────────────────────────────────────────────

TEST(ToolTest, GetPriceReturnsBasePrice) {
    ConcreteTool t(1, "Watering Can", "Waters crops", 25.0, 100);
    EXPECT_DOUBLE_EQ(t.getPrice(), 25.0);
}

TEST(ToolTest, GetDurabilityReturnsCorrectValue) {
    ConcreteTool t(1, "Hoe", "Tills soil", 15.0, 50);
    EXPECT_EQ(t.getDurability(), 50);
}

TEST(ToolTest, GetNameReturnsCorrectName) {
    ConcreteTool t(2, "Scythe", "Harvests crops", 40.0, 75);
    EXPECT_EQ(t.getName(), "Scythe");
}

TEST(ToolTest, UseCallsOverride) {
    ConcreteTool t(1, "Hoe", "Tills", 10.0, 30);
    Cell cell;
    Player player;
    t.use(cell, player);
    EXPECT_EQ(t.useCalls, 1);
}

TEST(ToolTest, UseCanBeCalledMultipleTimes) {
    ConcreteTool t(1, "Hoe", "Tills", 10.0, 30);
    Cell cell;
    Player player;
    t.use(cell, player);
    t.use(cell, player);
    t.use(cell, player);
    EXPECT_EQ(t.useCalls, 3);
}
