#include <gtest/gtest.h>

#include "../src/core/Player.h"
#include "../src/entities/HarvestedItem.h"
#include "../src/game/PlantFactory.h"
#include "../src/game/ShopData.h"
#include "../src/items/FertilizerTool.h"
#include "../src/items/WateringCan.h"
#include "../src/world/Cell.h"

#include <memory>
#include <stdexcept>
#include <string>

namespace {
ShopItemDef makeSeedDef(const std::string& name = "Carrot Seed",
                        const std::string& cropName = "Carrot",
                        float sellPrice = 30.0f,
                        int growTicks = 50,
                        int maxStages = 5,
                        bool regrows = false,
                        int regrowStage = 0)
{
    return {
        name,
        cropName,
        "Test seed",
        10.0f,
        sellPrice,
        growTicks,
        maxStages,
        regrows,
        regrowStage,
        sf::Color::White,
        sf::Color::Green,
        ShopItemType::SEED,
        0
    };
}
} // namespace

TEST(FertilizerToolTest, HasCurrentCatalogueStats) {
    FertilizerTool fertilizer;

    EXPECT_EQ(fertilizer.id(), 2u);
    EXPECT_EQ(fertilizer.getName(), "Fertilizer");
    EXPECT_DOUBLE_EQ(fertilizer.getPrice(), 50.0);
    EXPECT_EQ(fertilizer.getDurability(), 25);
    EXPECT_EQ(FertilizerTool::GROWTH_BOOST_TICKS, 20);
}

TEST(FertilizerToolTest, UseAdvancesPlantByTwentyTicks) {
    Cell cell;
    Player player;
    FertilizerTool fertilizer;

    ASSERT_TRUE(cell.setPlant(std::make_unique<Plant>(
        1, "Carrot", 0, 5, 10, 0, 30.0, false, 0)));

    fertilizer.use(cell, player);
    Plant* plant = cell.getPlant();

    ASSERT_NE(plant, nullptr);
    EXPECT_EQ(plant->getStage(), 2);
    EXPECT_EQ(plant->getTimeToGrowth(), 30u);
}

TEST(FertilizerToolTest, RepeatedUseCanFullyGrowPlant) {
    Cell cell;
    Player player;
    FertilizerTool fertilizer;

    ASSERT_TRUE(cell.setPlant(std::make_unique<Plant>(
        1, "Carrot", 0, 5, 10, 0, 30.0, false, 0)));

    fertilizer.use(cell, player);
    fertilizer.use(cell, player);
    EXPECT_FALSE(cell.getPlant()->isFullyGrown());

    fertilizer.use(cell, player);
    EXPECT_TRUE(cell.getPlant()->isFullyGrown());
}

TEST(ToolCatalogueTest, ToolBoostsMatchConcreteToolConstants) {
    const auto catalogue = makeShopCatalogue();
    bool foundWateringCan = false;
    bool foundFertilizer = false;

    for (const auto& item : catalogue) {
        if (item.name == "Watering Can") {
            foundWateringCan = true;
            EXPECT_EQ(item.type, ShopItemType::TOOL);
            EXPECT_EQ(item.toolBoost, WateringCan::GROWTH_BOOST_TICKS);
        }
        if (item.name == "Fertilizer") {
            foundFertilizer = true;
            EXPECT_EQ(item.type, ShopItemType::TOOL);
            EXPECT_EQ(item.toolBoost, FertilizerTool::GROWTH_BOOST_TICKS);
        }
    }

    EXPECT_TRUE(foundWateringCan);
    EXPECT_TRUE(foundFertilizer);
}

TEST(PlantFactoryTest, CreatesPlantFromSeedDefinition) {
    const auto plant = makePlant(makeSeedDef("Blueberry Seed", "Blueberry", 45.0f, 32, 4, true, 1));

    ASSERT_NE(plant, nullptr);
    EXPECT_EQ(plant->getName(), "Blueberry");
    EXPECT_EQ(plant->getStage(), 0);
    EXPECT_EQ(plant->getMaxStages(), 4);
    EXPECT_EQ(plant->getTimeToGrowth(), 32u);
}

TEST(PlantFactoryTest, UsesAtLeastOneTickPerStageForTinyGrowDurations) {
    const auto plant = makePlant(makeSeedDef("Fast Seed", "Fast Crop", 5.0f, 2, 5));

    ASSERT_NE(plant, nullptr);
    EXPECT_EQ(plant->getTimeToGrowth(), 5u);
}

TEST(PlantFactoryTest, HarvestedPlantUsesCatalogueSellPrice) {
    auto plant = makePlant(makeSeedDef("Rose Seed", "Rose", 80.0f, 72, 6));

    plant->grow(72);
    ASSERT_TRUE(plant->isFullyGrown());
    EXPECT_DOUBLE_EQ(plant->harvest().getPrice(), 80.0);
}

TEST(PlantFactoryTest, RegrowDataComesFromCatalogueDefinition) {
    auto plant = makePlant(makeSeedDef("Tomato Seed", "Tomato", 65.0f, 60, 6, true, 2));

    plant->grow(60);
    ASSERT_TRUE(plant->isFullyGrown());
    plant->harvest();

    EXPECT_EQ(plant->getStage(), 2);
    EXPECT_EQ(plant->getTimeToGrowth(), 40u);
}

TEST(PlantFactoryTest, RejectsNonSeedDefinitions) {
    auto toolDef = makeSeedDef("Fertilizer", "Fertilizer");
    toolDef.type = ShopItemType::TOOL;

    EXPECT_THROW(makePlant(toolDef), std::runtime_error);
}
