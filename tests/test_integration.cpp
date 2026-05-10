// ============================================================================
// test_integration.cpp
//
// Integration tests: verifies how the game classes interact end-to-end as
// they would in the main game loop. Each test group mirrors a real gameplay
// scenario rather than testing a single class in isolation.
// ============================================================================

#include <gtest/gtest.h>
#include <memory>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <thread>

// ── Game systems ─────────────────────────────────────────────────────────
#include "../src/core/Game.h"
#include "../src/core/Player.h"
#include "../src/core/TickSystem.h"
#include "../src/game/HarvestBasket.h"

// ── World ─────────────────────────────────────────────────────────────────
#include "../src/world/Garden.h"
#include "../src/world/Cell.h"
#include "../src/world/WeatherSystem.h"

// ── Concrete plant & item under test ──────────────────────────────────────
#include "../src/entities/HarvestedItem.h"
#include "../src/entities/Mutation.h"
#include "../src/items/WateringCan.h"
#include "../src/items/Seed.h"
#include "../src/systems/Shop.h"
#include "../src/systems/Inventory.h"
#include "../src/systems/RaccoonEvent.h"
#include "../src/systems/RandomEventManager.h"

namespace {
std::int64_t currentEpochSecondsForTest() {
    const auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
}

class SaveFileGuard {
public:
    SaveFileGuard() {
        std::ifstream in(path_, std::ios::binary);
        if (in.is_open()) {
            hadSave_ = true;
            std::ostringstream buffer;
            buffer << in.rdbuf();
            contents_ = buffer.str();
            in.close();
        }

        std::error_code ec;
        std::filesystem::remove(path_, ec);
    }

    ~SaveFileGuard() {
        std::error_code ec;
        if (hadSave_) {
            std::ofstream out(path_, std::ios::binary | std::ios::trunc);
            out << contents_;
        } else {
            std::filesystem::remove(path_, ec);
        }
    }

private:
    static constexpr const char* path_ = "save.json";
    bool hadSave_ = false;
    std::string contents_;
};

void writeDirtySaveWithCoconutAtOrigin() {
    nlohmann::json save = {
        {"player", {{"sheckles", 999}}},
        {"inventory", {{"items", nlohmann::json::array()}}},
        {"garden", {{"plants", nlohmann::json::array({
            {
                {"x", 0},
                {"y", 0},
                {"name", "Coconut"},
                {"stage", 0},
                {"maxStages", 7},
                {"ticksElapsed", 0}
            }
        })}}},
        {"game", {{"tick", 33}, {"initialized", true}}},
        {"harvestBasket", nlohmann::json::array()}
    };

    std::ofstream out("save.json");
    out << save.dump(2);
}
} // namespace

// ─────────────────────────────────────────────────────────────────────────
// GROUP 1 — Carrot full grow → harvest → sell lifecycle
// ─────────────────────────────────────────────────────────────────────────

// The most basic happy-path: plant a carrot, grow it to maturity, harvest,
// sell the HarvestedItem, and verify the player's sheckle balance updates.
TEST(Integration_CarrotLifecycle, PlantGrowHarvestSell) {
    Garden  garden(20, 20);
    Player  player;
    Shop    shop;

    player.addSheckles(0.0f);  // start broke — sale will add money

    // Plant a carrot at (2, 3)
    auto carrot = std::make_unique<Plant>(1, "Carrot", 0, 5, 10, 0, 30.0, false, 0);
    ASSERT_TRUE(garden.plantCrop(2, 3, std::move(carrot)));

    // Retrieve it and grow to full maturity (5 stages × 10 ticks = 50)
    Plant* plant = garden.getCell(2, 3).getPlant();
    ASSERT_NE(plant, nullptr) << "Cell should contain a Carrot";

    plant->grow(50);
    EXPECT_TRUE(plant->isFullyGrown());

    // Harvest
    HarvestedItem item = plant->harvest();
    EXPECT_DOUBLE_EQ(item.getPrice(), 30.0);  // base price, no mutations

    // Sell to shop
    float shecklesBefore = player.getSheckles();
    shop.processSale(std::make_unique<HarvestedItem>(item), &player);
    EXPECT_FLOAT_EQ(player.getSheckles(), shecklesBefore + 30.0f);
}

// A plant that regrows after harvest should reset to its regrow stage and
// be ready to grow again without needing to be replanted.
TEST(Integration_CarrotLifecycle, RegrowCarrotCyclesTwice) {
    // Use Plant directly to test regrow with custom stats
    auto carrot = std::make_unique<Plant>(1, "Carrot", 0, 5, 10, 0, 30.0, true, 1);
    Plant* c = carrot.get();

    // First harvest
    c->grow(50);
    ASSERT_TRUE(c->isFullyGrown());
    HarvestedItem first = c->harvest();
    EXPECT_DOUBLE_EQ(first.getPrice(), 30.0);
    EXPECT_FALSE(c->isFullyGrown());  // reset to stage 1

    // Second harvest — grow the remaining stages
    c->grow(40);  // (5-1) stages × 10 ticks
    EXPECT_TRUE(c->isFullyGrown());
    HarvestedItem second = c->harvest();
    EXPECT_DOUBLE_EQ(second.getPrice(), 30.0);
}

// Harvesting an unripe carrot should return a zero-value item and leave the
// plant untouched in its cell.
TEST(Integration_CarrotLifecycle, HarvestUnripeCarrotReturnsNothing) {
    Garden garden(20, 20);
    auto carrot = std::make_unique<Plant>(1, "Carrot", 0, 5, 10, 0, 30.0, false, 0);
    garden.plantCrop(0, 0, std::move(carrot));

    Plant* c = garden.getCell(0, 0).getPlant();
    c->grow(20);  // only partway grown

    HarvestedItem item = c->harvest();
    EXPECT_DOUBLE_EQ(item.getPrice(), 0.0);
    EXPECT_FALSE(c->isFullyGrown());   // plant unchanged
    EXPECT_NE(garden.getCell(0, 0).getPlant(), nullptr);  // still in cell
}

// ─────────────────────────────────────────────────────────────────────────
// GROUP 2 — Weather → Mutation pipeline
// ─────────────────────────────────────────────────────────────────────────

// RAIN weather (×2 WET mutation) applied before harvest should double the
// carrot's sell price.
TEST(Integration_WeatherMutation, RainMutationDoublesCarrotPrice) {
    Garden garden(20, 20);
    auto carrot = std::make_unique<Plant>(1, "Carrot", 0, 5, 10, 0, 30.0, false, 0);
    garden.plantCrop(0, 0, std::move(carrot));

    Plant* c = garden.getCell(0, 0).getPlant();

    // Manually apply WET mutation (as WeatherSystem would do with RAIN)
    c->addMutation(Mutation(MutationType::WET, 2.0f, WeatherType::RAIN));

    c->grow(50);
    HarvestedItem item = c->harvest();

    EXPECT_DOUBLE_EQ(item.getPrice(), 60.0);  // 30 × 2
    ASSERT_EQ(item.getMutationList().size(), 1u);
    EXPECT_EQ(item.getMutationList()[0], MutationType::WET);
}

// Current mutation pricing adds each multiplier contribution:
// FROST and SHOCKED on a 30-sheckle carrot gives 30×5 + 30×100 = 3150.
TEST(Integration_WeatherMutation, StackedMutationsAddMultiplierContributions) {
    Plant c(1, "Carrot", 0, 5, 10, 0, 30.0, false, 0);
    c.addMutation(Mutation(MutationType::FROZEN,  5.0f,   WeatherType::FROST));
    c.addMutation(Mutation(MutationType::SHOCKED, 100.0f, WeatherType::THUNDER_STORM));
    c.grow(50);
    EXPECT_DOUBLE_EQ(c.harvest().getPrice(), 3150.0);
}

// METEOR_SHOWER (×150 CELESTIAL) is the highest multiplier in the game.
TEST(Integration_WeatherMutation, CelestialMutationGivesMaxMultiplier) {
    Plant c(1, "Carrot", 0, 5, 10, 0, 30.0, false, 0);
    c.addMutation(Mutation(MutationType::CELESTIAL, 150.0f, WeatherType::METEOR_SHOWER));
    c.grow(50);
    EXPECT_DOUBLE_EQ(c.harvest().getPrice(), 30.0 * 150.0);
}

// WeatherSystem::applyEffectsToGrid should touch every occupied cell and
// adjust the carrot's growth speed when RAIN is active.
TEST(Integration_WeatherMutation, WeatherSystemSpeedsUpCarrotGrowth) {
    Garden garden(20, 20);
    garden.plantCrop(5, 5, std::make_unique<Plant>(1, "Carrot", 0, 5, 10, 0, 30.0, false, 0));
    Plant* c = garden.getCell(5, 5).getPlant();

    WeatherSystem ws;
    // Apply SUMMER (×1.0) so we know the baseline, then switch to RAIN growth.
    // We cannot force WeatherSystem's internal weather, so we call
    // applyWeatherEffect directly on the plant — the same call that
    // applyEffectsToGrid makes internally.
    c->applyWeatherEffect(WeatherType::RAIN);

    // With RAIN: round(10/1.5)=7 ticks/stage → 35 ticks to fully grow.
    // Without RAIN it would take 50. Verify 35 ticks suffices.
    c->grow(35);
    EXPECT_TRUE(c->isFullyGrown());
}

// ─────────────────────────────────────────────────────────────────────────
// GROUP 3 — Shop economy
// ─────────────────────────────────────────────────────────────────────────

// Player buys a Carrot Seed from the shop → sheckles deducted.
TEST(Integration_ShopEconomy, PlayerBuysCarrotSeed) {
    Shop   shop;
    Player player;
    player.addSheckles(100.0f);

    auto seed = std::make_unique<Seed>(1, "Carrot Seed", "Grows a carrot", 10.0, "Carrot");
    shop.addAvailableItem(std::move(seed));

    Item* seedPtr = shop.getAvailableItems().front().get();
    bool bought = player.buy(seedPtr, &shop);

    EXPECT_TRUE(bought);
    EXPECT_FLOAT_EQ(player.getSheckles(), 90.0f);
    EXPECT_EQ(player.getInventory().getQuantity("Carrot Seed"), 1);
}

// Player cannot buy if they don't have enough sheckles.
TEST(Integration_ShopEconomy, PlayerCannotBuyWhenBroke) {
    Shop   shop;
    Player player;
    player.addSheckles(5.0f);

    auto seed = std::make_unique<Seed>(1, "Carrot Seed", "desc", 10.0, "Carrot");
    shop.addAvailableItem(std::move(seed));

    Item* seedPtr = shop.getAvailableItems().front().get();
    EXPECT_FALSE(player.buy(seedPtr, &shop));
    EXPECT_FLOAT_EQ(player.getSheckles(), 5.0f);  // unchanged
    EXPECT_EQ(player.getInventory().getQuantity("Carrot Seed"), 0);
}

TEST(Integration_ShopEconomy, PlayerBuysWateringCanThroughShopApi) {
    Shop shop;
    Player player;
    player.addSheckles(100.0f);

    shop.addAvailableItem(std::make_unique<WateringCan>());
    ASSERT_FALSE(shop.getAvailableItems().empty());

    EXPECT_TRUE(player.buy(shop.getAvailableItems().front().get(), &shop));
    EXPECT_FLOAT_EQ(player.getSheckles(), 75.0f);
    EXPECT_EQ(player.getInventory().getQuantity("Watering Can"), 1);
}

// Selling a harvested carrot via the shop adds its price to player's sheckles.
TEST(Integration_ShopEconomy, SellingHarvestedCarrotAddsSheckles) {
    Shop   shop;
    Player player;
    player.addSheckles(0.0f);

    Plant c(1, "Carrot", 0, 5, 10, 0, 30.0, false, 0);
    c.grow(50);
    HarvestedItem item = c.harvest();
    double price = item.getPrice();  // 30.0

    shop.processSale(std::make_unique<HarvestedItem>(item), &player);
    EXPECT_FLOAT_EQ(player.getSheckles(), static_cast<float>(price));
}

TEST(Integration_ShopEconomy, SellingClonedHarvestedItemAddsSheckles) {
    Shop shop;
    Player player;
    HarvestedItem item(45.0, {MutationType::WET});

    EXPECT_TRUE(shop.processSale(item.clone(), &player));
    EXPECT_FLOAT_EQ(player.getSheckles(), 45.0f);
    EXPECT_DOUBLE_EQ(item.getPrice(), 45.0);
}

// Sell a mutation-boosted carrot — shop adds the mutated price.
TEST(Integration_ShopEconomy, SellingMutatedCarrotAddsCorrectSheckles) {
    Shop   shop;
    Player player;
    player.addSheckles(0.0f);

    Plant c(1, "Carrot", 0, 5, 10, 0, 30.0, false, 0);
    c.addMutation(Mutation(MutationType::WET, 2.0f, WeatherType::RAIN));
    c.grow(50);
    HarvestedItem item = c.harvest();  // 30 × 2 = 60

    shop.processSale(std::make_unique<HarvestedItem>(item), &player);
    EXPECT_FLOAT_EQ(player.getSheckles(), 60.0f);
}

// ─────────────────────────────────────────────────────────────────────────
// GROUP 4 — Player inventory flow
// ─────────────────────────────────────────────────────────────────────────

// Player receives seeds into their inventory, uses them to plant, then
// the inventory count drops.
TEST(Integration_Inventory, PlayerStoresAndConsumesSeeds) {
    Player player;
    Inventory& inv = player.getInventory();

    // Add 3 carrot seeds to inventory
    inv.addItem(std::make_unique<Seed>(1, "Carrot Seed", "desc", 10.0, "Carrot"), 3);
    EXPECT_EQ(inv.getQuantity("Carrot Seed"), 3);

    // Plant two of them (consume from inventory)
    EXPECT_TRUE(inv.removeItem("Carrot Seed", 2));
    EXPECT_EQ(inv.getQuantity("Carrot Seed"), 1);
}

// Inventory slot cap is respected — 20 slots by default for a Player.
TEST(Integration_Inventory, InventoryDoesNotExceedSlotCap) {
    Inventory inv(2);  // tiny inventory for test speed
    inv.addItem(std::make_unique<Seed>(1, "A", "d", 1.0, "A"), 1);
    inv.addItem(std::make_unique<Seed>(2, "B", "d", 1.0, "B"), 1);
    EXPECT_TRUE(inv.isFull());

    bool added = inv.addItem(std::make_unique<Seed>(3, "C", "d", 1.0, "C"), 1);
    EXPECT_FALSE(added);
}

// ─────────────────────────────────────────────────────────────────────────
// GROUP 5 — WateringCan Tool interaction
// ─────────────────────────────────────────────────────────────────────────

// Equipping the WateringCan and using it on a cell should advance the
// Carrot's growth by GROWTH_BOOST_TICKS without needing a manual grow().
TEST(Integration_WateringCan, EquipAndUseAdvancesCarrotGrowth) {
    Garden garden(20, 20);
    Player player;
    garden.plantCrop(0, 0, std::make_unique<Plant>(1, "Carrot", 0, 5, 10, 0, 30.0, false, 0));

    // Equip watering can
    auto can = std::make_shared<WateringCan>();
    player.equipTool(can);

    // Use it 10 times → 10 × 5 = 50 ticks → fully grown (5×10)
    Cell& cell = garden.getCell(0, 0);
    for (int i = 0; i < 10; ++i) {
        player.useTool(cell);
    }

    Plant* c = cell.getPlant();
    ASSERT_NE(c, nullptr);
    EXPECT_TRUE(c->isFullyGrown());
}

// Using the tool on an empty cell should not crash.
TEST(Integration_WateringCan, UseOnEmptyCellDoesNotCrash) {
    Cell cell;
    Player player;
    auto can = std::make_shared<WateringCan>();
    player.equipTool(can);
    EXPECT_NO_THROW(player.useTool(cell));
}

// Tool price and durability are accessible externally (via public using declarations).
TEST(Integration_WateringCan, ToolAttributesAreReadable) {
    WateringCan can;
    EXPECT_DOUBLE_EQ(can.getPrice(), 25.0);
    EXPECT_EQ(can.getDurability(), 50);
    EXPECT_EQ(can.getName(), "Watering Can");
}

// ─────────────────────────────────────────────────────────────────────────
// GROUP 6 — TickSystem driving plant growth
// ─────────────────────────────────────────────────────────────────────────

// Simulated game loop: update() advances the TickSystem; once enough ticks
// accumulate the plant would be considered grown. We test tick counting here;
// wiring grow() into the tick loop is main.cpp's responsibility.
TEST(Integration_TickSystem, TickSystemAccumulatesTicks) {
    TickSystem ts(1.0f);  // 1 tick per second

    ts.update(0.5f);
    EXPECT_EQ(ts.getTick(), 0u);  // not enough time yet

    ts.update(0.5f);
    EXPECT_EQ(ts.getTick(), 1u);  // crossed 1-second boundary

    ts.update(10.0f);
    EXPECT_EQ(ts.getTick(), 11u);
}

TEST(Integration_TickSystem, ResetPreservesTickRateAndClearsAccumulator) {
    TickSystem ts(0.5f);
    ts.update(0.25f);
    ts.fastForward(10);

    ts.reset(3);
    EXPECT_EQ(ts.getTick(), 3u);

    ts.update(0.25f);
    EXPECT_EQ(ts.getTick(), 3u);

    ts.update(0.25f);
    EXPECT_EQ(ts.getTick(), 4u);
}

// fastForward simulates the offline-progress path: bulk ticks as if the
// player came back after being away.
TEST(Integration_TickSystem, FastForwardThenGrowCarrot) {
    TickSystem ts(1.0f);
    ts.fastForward(50);
    EXPECT_EQ(ts.getTick(), 50u);

    // In real game loop these ticks would be fed to grow() during load.
    // Here we verify that 50 ticks is enough to fully grow a default Carrot.
    Plant c(1, "Carrot", 0, 5, 10, 0, 30.0, false, 0);
    c.grow(ts.getTick());
    EXPECT_TRUE(c.isFullyGrown());
}

// ─────────────────────────────────────────────────────────────────────────
// GROUP 7 — RaccoonEvent steals from a populated garden
// ─────────────────────────────────────────────────────────────────────────

static void fillGardenWithCarrots(Garden& g) {
    for (int y = 0; y < g.getHeight(); ++y)
        for (int x = 0; x < g.getWidth(); ++x)
            g.plantCrop(x, y, std::make_unique<Plant>(1, "Carrot", 0, 5, 10, 0, 30.0, false, 0));
}

static int countOccupied(const Garden& g) {
    int n = 0;
    for (int y = 0; y < g.getHeight(); ++y)
        for (int x = 0; x < g.getWidth(); ++x)
            if (!g.getCell(x, y).isEmpty()) ++n;
    return n;
}

TEST(Integration_RaccoonEvent, StealsPlantsFromPopulatedGarden) {
    Garden garden(20, 20);
    Player player;
    fillGardenWithCarrots(garden);

    RaccoonEvent raccoon(10);
    raccoon.trigger(garden, player);

    int remaining = countOccupied(garden);
    EXPECT_LE(remaining, 400 - 1)  // at least 1 stolen
        << "Raccoon should have stolen at least one carrot";
    EXPECT_GE(remaining, 400 - 10) // at most 10 stolen
        << "Raccoon should not steal more than its stolenAmount";
}

TEST(Integration_RaccoonEvent, EmptyGardenDoesNotCrash) {
    Garden garden(20, 20);
    Player player;
    RaccoonEvent raccoon(5);
    EXPECT_NO_THROW(raccoon.trigger(garden, player));
}

// ─────────────────────────────────────────────────────────────────────────
// GROUP 8 — RandomEventManager schedules raccoon against a live garden
// ─────────────────────────────────────────────────────────────────────────

TEST(Integration_RandomEventManager, RaccoonFiredByManagerStealsCrops) {
    Garden garden(20, 20);
    Player player;
    fillGardenWithCarrots(garden);

    int before = countOccupied(garden);

    RandomEventManager mgr(1.0f);
    mgr.registerEvent(std::make_unique<RaccoonEvent>(5));
    mgr.update(61.0f, garden, player);  // 61 seconds triggers one event

    EXPECT_LT(countOccupied(garden), before);
}

TEST(Integration_RandomEventManager, TriggerIntervalPreventsImmediateRepeat) {
    Garden garden(20, 20);
    Player player;
    fillGardenWithCarrots(garden);

    RandomEventManager mgr(1.0f);
    mgr.registerEvent(std::make_unique<RaccoonEvent>(1));

    mgr.update(61.0f, garden, player);  // fires once
    int afterFirst = countOccupied(garden);

    mgr.update(30.0f, garden, player);  // only 30 seconds more, below 60s interval
    EXPECT_EQ(countOccupied(garden), afterFirst);  // no second steal
}

// ─────────────────────────────────────────────────────────────────────────
// GROUP 9 — Game object top-level integration
// ─────────────────────────────────────────────────────────────────────────

// Game can be constructed and update() can be called without crashing.
TEST(Integration_Game, ConstructAndUpdateDoesNotCrash) {
    SaveFileGuard saveGuard;
    Game game;
    EXPECT_NO_THROW(game.update(0.016f));  // ~60 fps frame
    EXPECT_NO_THROW(game.update(1.0f));
}

// Planting a carrot via Game::getGarden() and growing it via the tick system.
TEST(Integration_Game, PlantAndGrowCarrotThroughGameInterface) {
    SaveFileGuard ambientSaveGuard;
    writeDirtySaveWithCoconutAtOrigin();

    SaveFileGuard saveGuard;
    Game game;
    Garden& garden = game.getGarden();

    ASSERT_TRUE(garden.plantCrop(0, 0, std::make_unique<Plant>(1, "Carrot", 0, 5, 10, 0, 30.0, false, 0)));
    ASSERT_NE(garden.getCell(0, 0).getPlant(), nullptr);
    EXPECT_EQ(garden.getCell(0, 0).getPlant()->getName(), "Carrot");

    // Fast-forward the tick system to simulate 50 seconds of play
    game.getTickSystem().fastForward(50);

    // Manually apply ticks to plant (in a real game loop Game::update would do this)
    Plant* c = garden.getCell(0, 0).getPlant();
    ASSERT_NE(c, nullptr);
    c->grow(game.getTickSystem().getTick());

    EXPECT_TRUE(c->isFullyGrown());
}

// processOfflineProgress fast-forwards ticks when called after a save.
TEST(Integration_Game, OfflineProgressFastForwardsTicks) {
    SaveFileGuard saveGuard;
    Game game;
    game.saveGame();

    std::size_t ticksBefore = game.getTickSystem().getTick();

    // Simulate offline time by calling processOfflineProgress.
    // In real usage, time passes between saveGame() and loadGame().
    // Since we cannot truly wait, we just verify it does not crash and
    // tick count does not decrease.
    game.processOfflineProgress();
    EXPECT_GE(game.getTickSystem().getTick(), ticksBefore);
}

TEST(Integration_Game, SaveGameWritesSaveTimestamp) {
    SaveFileGuard saveGuard;
    const std::int64_t beforeSave = currentEpochSecondsForTest();

    Game game;
    game.saveGame();

    std::ifstream in("save.json");
    nlohmann::json saved;
    in >> saved;

    ASSERT_TRUE(saved.contains("game"));
    ASSERT_TRUE(saved["game"].contains("saveTimestamp"));
    ASSERT_TRUE(saved["game"]["saveTimestamp"].is_number_integer() ||
                saved["game"]["saveTimestamp"].is_number_unsigned());

    const std::int64_t saveTimestamp = saved["game"]["saveTimestamp"].get<std::int64_t>();
    const std::int64_t afterSave = currentEpochSecondsForTest();
    EXPECT_GE(saveTimestamp, beforeSave);
    EXPECT_LE(saveTimestamp, afterSave);
}

TEST(Integration_Game, SaveGameWritesGardenPlantMutations) {
    SaveFileGuard saveGuard;
    Game game;
    Garden& garden = game.getGarden();

    ASSERT_TRUE(garden.plantCrop(0, 0, std::make_unique<Plant>(1, "Carrot", 0, 5, 10, 0, 30.0, false, 0)));
    Plant* carrot = garden.getCell(0, 0).getPlant();
    ASSERT_NE(carrot, nullptr);
    carrot->addMutation(Mutation(MutationType::WET, 2.0f, WeatherType::RAIN));
    carrot->addMutation(Mutation(MutationType::FROZEN, 5.0f, WeatherType::FROST));

    game.saveGame();

    std::ifstream in("save.json");
    nlohmann::json saved;
    in >> saved;

    ASSERT_TRUE(saved.contains("garden"));
    ASSERT_TRUE(saved["garden"].contains("plants"));
    ASSERT_TRUE(saved["garden"]["plants"].is_array());
    ASSERT_EQ(saved["garden"]["plants"].size(), 1u);

    const auto& mutations = saved["garden"]["plants"][0]["mutations"];
    ASSERT_TRUE(mutations.is_array());
    ASSERT_EQ(mutations.size(), 2u);
    EXPECT_EQ(mutations[0].get<int>(), static_cast<int>(MutationType::WET));
    EXPECT_EQ(mutations[1].get<int>(), static_cast<int>(MutationType::FROZEN));
}

TEST(Integration_Game, LoadGameRestoresGardenPlantMutations) {
    SaveFileGuard saveGuard;
    nlohmann::json save = {
        {"player", {{"sheckles", 0}}},
        {"inventory", {{"items", nlohmann::json::array()}}},
        {"garden", {{"plants", nlohmann::json::array({
            {
                {"x", 0},
                {"y", 0},
                {"name", "Carrot"},
                {"stage", 0},
                {"maxStages", 5},
                {"ticksElapsed", 0},
                {"mutations", nlohmann::json::array({
                    static_cast<int>(MutationType::WET),
                    static_cast<int>(MutationType::FROZEN)
                })}
            }
        })}}},
        {"game", {{"tick", 0}, {"initialized", true}, {"saveTimestamp", currentEpochSecondsForTest()}}},
        {"harvestBasket", nlohmann::json::array()}
    };

    std::ofstream out("save.json");
    out << save.dump(2);
    out.close();

    Game game;
    Plant* carrot = game.getGarden().getCell(0, 0).getPlant();
    ASSERT_NE(carrot, nullptr);

    const std::vector<MutationType> mutations = carrot->getMutations();
    ASSERT_EQ(mutations.size(), 2u);
    EXPECT_EQ(mutations[0], MutationType::WET);
    EXPECT_EQ(mutations[1], MutationType::FROZEN);

    carrot->grow(50);
    EXPECT_DOUBLE_EQ(carrot->harvest().getPrice(), 210.0);
}

TEST(Integration_Game, LoadGameAcceptsGardenPlantWithoutMutations) {
    SaveFileGuard saveGuard;
    nlohmann::json save = {
        {"player", {{"sheckles", 0}}},
        {"inventory", {{"items", nlohmann::json::array()}}},
        {"garden", {{"plants", nlohmann::json::array({
            {
                {"x", 0},
                {"y", 0},
                {"name", "Carrot"},
                {"stage", 0},
                {"maxStages", 5},
                {"ticksElapsed", 0}
            }
        })}}},
        {"game", {{"tick", 0}, {"initialized", true}, {"saveTimestamp", currentEpochSecondsForTest()}}},
        {"harvestBasket", nlohmann::json::array()}
    };

    std::ofstream out("save.json");
    out << save.dump(2);
    out.close();

    Game game;
    Plant* carrot = game.getGarden().getCell(0, 0).getPlant();
    ASSERT_NE(carrot, nullptr);
    EXPECT_TRUE(carrot->getMutations().empty());
}

TEST(Integration_Game, LoadGameKeepsPlantWhenGardenPlantMutationsAreMalformed) {
    SaveFileGuard saveGuard;
    nlohmann::json save = {
        {"player", {{"sheckles", 0}}},
        {"inventory", {{"items", nlohmann::json::array()}}},
        {"garden", {{"plants", nlohmann::json::array({
            {
                {"x", 0},
                {"y", 0},
                {"name", "Carrot"},
                {"stage", 0},
                {"maxStages", 5},
                {"ticksElapsed", 0},
                {"mutations", nlohmann::json::array({0, "bad"})}
            }
        })}}},
        {"game", {{"tick", 0}, {"initialized", true}, {"saveTimestamp", currentEpochSecondsForTest()}}},
        {"harvestBasket", nlohmann::json::array()}
    };

    std::ofstream out("save.json");
    out << save.dump(2);
    out.close();

    Game game;
    Plant* carrot = game.getGarden().getCell(0, 0).getPlant();
    ASSERT_NE(carrot, nullptr);
    EXPECT_EQ(carrot->getName(), "Carrot");
    EXPECT_TRUE(carrot->getMutations().empty());
}

TEST(Integration_Game, LoadGameAppliesOfflineGrowthToSavedPlants) {
    SaveFileGuard saveGuard;
    const std::int64_t savedAt = currentEpochSecondsForTest() - 60;
    nlohmann::json save = {
        {"player", {{"sheckles", 0}}},
        {"inventory", {{"items", nlohmann::json::array()}}},
        {"garden", {{"plants", nlohmann::json::array({
            {
                {"x", 0},
                {"y", 0},
                {"name", "Carrot"},
                {"stage", 0},
                {"maxStages", 5},
                {"ticksElapsed", 0}
            }
        })}}},
        {"game", {{"tick", 0}, {"initialized", true}, {"saveTimestamp", savedAt}}},
        {"harvestBasket", nlohmann::json::array()}
    };

    std::ofstream out("save.json");
    out << save.dump(2);
    out.close();

    Game game;
    Garden& garden = game.getGarden();

    EXPECT_EQ(garden.getWidth(), 5);
    EXPECT_EQ(garden.getHeight(), 4);

    Plant* carrot = garden.getCell(0, 0).getPlant();
    ASSERT_NE(carrot, nullptr);
    EXPECT_EQ(carrot->getName(), "Carrot");
    EXPECT_TRUE(carrot->isFullyGrown());
    EXPECT_EQ(carrot->getTimeToGrowth(), 0u);
    EXPECT_GE(game.getTickSystem().getTick(), 60u);
}

TEST(Integration_Game, LoadGameReplacesExistingStateOnRepeatLoad) {
    SaveFileGuard saveGuard;
    const std::int64_t savedAt = currentEpochSecondsForTest() + 60;
    nlohmann::json save = {
        {"player", {{"sheckles", 42}}},
        {"inventory", {{"items", nlohmann::json::array({
            {{"name", "Carrot Seed"}, {"quantity", 2}}
        })}}},
        {"garden", {{"plants", nlohmann::json::array({
            {
                {"x", 1},
                {"y", 1},
                {"name", "Carrot"},
                {"stage", 0},
                {"maxStages", 5},
                {"ticksElapsed", 0}
            }
        })}}},
        {"game", {{"tick", 12}, {"initialized", true}, {"saveTimestamp", savedAt}}},
        {"harvestBasket", nlohmann::json::array({
            {{"cropName", "Carrot"}, {"price", 60.0}, {"mutations", nlohmann::json::array({0})}}
        })}
    };

    std::ofstream out("save.json");
    out << save.dump(2);
    out.close();

    Game game;
    std::vector<BasketEntry> basket;
    game.bindHarvestBasket(basket);

    game.getPlayer().addSheckles(100.0f);
    game.getPlayer().getInventory().addItem(
        std::make_unique<Seed>(1, "Blueberry Seed", "desc", 15.0, "Blueberry"), 7);
    ASSERT_TRUE(game.getGarden().plantCrop(
        0, 0, std::make_unique<Plant>(1, "Coconut", 0, 7, 25, 0, 200.0, false, 0)));
    game.getTickSystem().fastForward(100);
    basket.push_back({HarvestedItem(30.0, {}), "Dirty"});

    game.loadGame();

    EXPECT_TRUE(game.hasLoadedSave());
    EXPECT_TRUE(game.isInitialized());
    EXPECT_FLOAT_EQ(game.getPlayer().getSheckles(), 42.0f);
    EXPECT_EQ(game.getPlayer().getInventory().getQuantity("Carrot Seed"), 2);
    EXPECT_EQ(game.getPlayer().getInventory().getQuantity("Blueberry Seed"), 0);
    EXPECT_EQ(game.getTickSystem().getTick(), 12u);

    Garden& garden = game.getGarden();
    EXPECT_EQ(garden.getWidth(), 5);
    EXPECT_EQ(garden.getHeight(), 4);
    EXPECT_EQ(countOccupied(garden), 1);
    EXPECT_EQ(garden.getCell(0, 0).getPlant(), nullptr);
    Plant* savedPlant = garden.getCell(1, 1).getPlant();
    ASSERT_NE(savedPlant, nullptr);
    EXPECT_EQ(savedPlant->getName(), "Carrot");

    ASSERT_EQ(basket.size(), 1u);
    EXPECT_EQ(basket.front().cropName_, "Carrot");
    EXPECT_DOUBLE_EQ(basket.front().item_.getPrice(), 60.0);

    game.unbindHarvestBasket();
}

TEST(Integration_Game, LoadGameMissingOptionalSectionsClearsExistingState) {
    SaveFileGuard saveGuard;
    const std::int64_t savedAt = currentEpochSecondsForTest() + 60;
    nlohmann::json initialSave = {
        {"player", {{"sheckles", 99}}},
        {"inventory", {{"items", nlohmann::json::array({
            {{"name", "Carrot Seed"}, {"quantity", 3}}
        })}}},
        {"garden", {{"plants", nlohmann::json::array({
            {
                {"x", 0},
                {"y", 0},
                {"name", "Carrot"},
                {"stage", 0},
                {"maxStages", 5},
                {"ticksElapsed", 0}
            }
        })}}},
        {"game", {{"tick", 8}, {"initialized", true}, {"saveTimestamp", savedAt}}},
        {"harvestBasket", nlohmann::json::array({
            {{"cropName", "Carrot"}, {"price", 30.0}, {"mutations", nlohmann::json::array()}}
        })}
    };

    std::ofstream initialOut("save.json");
    initialOut << initialSave.dump(2);
    initialOut.close();

    Game game;
    std::vector<BasketEntry> basket;
    game.bindHarvestBasket(basket);
    ASSERT_FALSE(basket.empty());

    nlohmann::json sparseSave = {
        {"game", {{"tick", 5}, {"initialized", false}, {"saveTimestamp", savedAt}}}
    };

    std::ofstream sparseOut("save.json");
    sparseOut << sparseSave.dump(2);
    sparseOut.close();

    game.loadGame();

    EXPECT_TRUE(game.hasLoadedSave());
    EXPECT_FALSE(game.isInitialized());
    EXPECT_FLOAT_EQ(game.getPlayer().getSheckles(), 0.0f);
    EXPECT_EQ(game.getPlayer().getInventory().getQuantity("Carrot Seed"), 0);
    EXPECT_EQ(countOccupied(game.getGarden()), 0);
    EXPECT_EQ(game.getTickSystem().getTick(), 5u);
    EXPECT_TRUE(basket.empty());

    game.unbindHarvestBasket();
}

TEST(Integration_Game, LoadGameWithoutSaveDoesNotWipeLiveState) {
    SaveFileGuard saveGuard;
    Game game;
    std::vector<BasketEntry> basket;
    game.bindHarvestBasket(basket);

    game.getPlayer().addSheckles(12.0f);
    game.getPlayer().getInventory().addItem(
        std::make_unique<Seed>(1, "Carrot Seed", "desc", 10.0, "Carrot"), 4);
    ASSERT_TRUE(game.getGarden().plantCrop(
        0, 0, std::make_unique<Plant>(1, "Carrot", 0, 5, 10, 0, 30.0, false, 0)));
    game.getTickSystem().fastForward(7);
    basket.push_back({HarvestedItem(30.0, {}), "Carrot"});

    game.loadGame();

    EXPECT_FALSE(game.hasLoadedSave());
    EXPECT_FLOAT_EQ(game.getPlayer().getSheckles(), 12.0f);
    EXPECT_EQ(game.getPlayer().getInventory().getQuantity("Carrot Seed"), 4);
    EXPECT_EQ(countOccupied(game.getGarden()), 1);
    EXPECT_EQ(game.getTickSystem().getTick(), 7u);
    ASSERT_EQ(basket.size(), 1u);
    EXPECT_EQ(basket.front().cropName_, "Carrot");

    game.unbindHarvestBasket();
}

TEST(Integration_Game, SaveGamePreservesHarvestBasketWhenUnbound) {
    SaveFileGuard saveGuard;
    nlohmann::json existing = {
        {"game", {{"initialized", true}, {"tick", 7}}},
        {"harvestBasket", nlohmann::json::array({
            {{"cropName", "Carrot"}, {"price", 30.0}, {"mutations", nlohmann::json::array({0})}}
        })},
        {"customTopLevel", "keep"}
    };

    std::ofstream out("save.json");
    out << existing.dump(2);
    out.close();

    Game game;
    game.getPlayer().addSheckles(25.0f);
    game.saveGame();

    std::ifstream in("save.json");
    nlohmann::json saved;
    in >> saved;

    EXPECT_EQ(saved["harvestBasket"], existing["harvestBasket"]);
    EXPECT_EQ(saved["customTopLevel"], "keep");
    EXPECT_TRUE(saved["game"].value("initialized", false));
    EXPECT_FLOAT_EQ(saved["player"]["sheckles"].get<float>(), 25.0f);
}

TEST(Integration_Game, BindHarvestBasketLoadsSavedBasket) {
    SaveFileGuard saveGuard;
    nlohmann::json existing = {
        {"game", {{"initialized", true}, {"tick", 0}}},
        {"harvestBasket", nlohmann::json::array({
            {{"cropName", "Carrot"}, {"price", 60.0}, {"mutations", nlohmann::json::array({0})}}
        })}
    };

    std::ofstream out("save.json");
    out << existing.dump(2);
    out.close();

    Game game;
    std::vector<BasketEntry> basket;
    game.bindHarvestBasket(basket);

    ASSERT_EQ(basket.size(), 1u);
    EXPECT_EQ(basket.front().cropName_, "Carrot");
    EXPECT_DOUBLE_EQ(basket.front().item_.getPrice(), 60.0);

    game.unbindHarvestBasket();
}

TEST(Integration_Game, BindHarvestBasketSkipsMalformedEntries) {
    SaveFileGuard saveGuard;
    nlohmann::json existing = {
        {"game", {{"initialized", true}, {"tick", 0}}},
        {"harvestBasket", nlohmann::json::array({
            "not an object",
            {{"cropName", ""}, {"price", 10.0}, {"mutations", nlohmann::json::array()}},
            {{"cropName", 123}, {"price", 10.0}, {"mutations", nlohmann::json::array()}},
            {{"cropName", "Rose"}, {"price", "expensive"}, {"mutations", nlohmann::json::array()}},
            {{"cropName", "Corn"}, {"price", 60.0}, {"mutations", "wet"}},
            {{"cropName", "Tomato"}, {"price", 65.0}, {"mutations", nlohmann::json::array({0, "bad"})}},
            {{"cropName", "Cactus"}, {"price", 90.0}, {"mutations", nlohmann::json::array({99})}},
            {{"cropName", "Blueberry"}, {"price", 45.0}, {"mutations", nlohmann::json::array({0, 2})}}
        })}
    };

    std::ofstream out("save.json");
    out << existing.dump(2);
    out.close();

    Game game;
    std::vector<BasketEntry> basket = {
        {HarvestedItem(1.0, {}), "Old"}
    };

    EXPECT_NO_THROW(game.bindHarvestBasket(basket));

    ASSERT_EQ(basket.size(), 1u);
    EXPECT_EQ(basket.front().cropName_, "Blueberry");
    EXPECT_DOUBLE_EQ(basket.front().item_.getPrice(), 45.0);
    EXPECT_EQ(basket.front().item_.getMutationList().size(), 2u);

    game.unbindHarvestBasket();
}

// Player sheckle balance survives a full buy → grow → sell cycle via Game.
TEST(Integration_Game, FullEconomyCycleThroughGameInterface) {
    SaveFileGuard saveGuard;
    Game  game;
    Shop  shop;
    Player& player = game.getPlayer();

    player.addSheckles(100.0f);

    // Buy seed
    auto seed = std::make_unique<Seed>(1, "Carrot Seed", "desc", 10.0, "Carrot");
    shop.addAvailableItem(std::move(seed));
    player.buy(shop.getAvailableItems().front().get(), &shop);
    EXPECT_FLOAT_EQ(player.getSheckles(), 90.0f);

    // Plant, grow, harvest, sell
    game.getGarden().plantCrop(0, 0, std::make_unique<Plant>(1, "Carrot", 0, 5, 10, 0, 30.0, false, 0));
    Plant* c = game.getGarden().getCell(0, 0).getPlant();
    c->grow(50);
    HarvestedItem item = c->harvest();
    shop.processSale(std::make_unique<HarvestedItem>(item), &player);

    // Started at 100 − 10 (seed) + 30 (harvest) = 120
    EXPECT_FLOAT_EQ(player.getSheckles(), 120.0f);
}
