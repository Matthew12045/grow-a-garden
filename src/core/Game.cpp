#include "Game.h"
#include "../game/HarvestBasket.h"
#include "../game/ShopData.h"
#include "../entities/Mutation.h"
#include "../systems/RaccoonEvent.h"
#include "../game/PlantFactory.h"
#include "../items/FertilizerTool.h"
#include "../items/Item.h"
#include "../items/Seed.h"
#include "../items/Tool.h"
#include "../items/WateringCan.h"
#include "../ui/DrawUtils.h"
#include <chrono>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <cerrno>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <stdexcept>
#include <utility>

using json = nlohmann::json;

namespace {
std::int64_t currentEpochSeconds() {
    const auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
}

const ShopItemDef* findDefByName(const std::vector<ShopItemDef>& catalogue, const std::string& name) {
    const auto it = std::find_if(catalogue.begin(), catalogue.end(), [&](const ShopItemDef& def) {
        return def.name == name;
    });
    return (it != catalogue.end()) ? &(*it) : nullptr;
}

const ShopItemDef* findDefByCropName(const std::vector<ShopItemDef>& catalogue, const std::string& cropName) {
    const auto it = std::find_if(catalogue.begin(), catalogue.end(), [&](const ShopItemDef& def) {
        return def.cropName == cropName;
    });
    return (it != catalogue.end()) ? &(*it) : nullptr;
}

std::unique_ptr<Item> makeInventoryItemFromDef(const ShopItemDef& def) {
    if (def.type == ShopItemType::SEED) {
        return std::make_unique<Seed>(1, def.name, def.description, def.buyPrice, def.cropName);
    }

    if (def.name == "Watering Can") {
        return std::make_unique<WateringCan>(def.buyPrice);
    }

    if (def.name == "Fertilizer") {
        return std::make_unique<FertilizerTool>();
    }

    return nullptr;
}

void restoreToolDurabilityFromSave(Tool& tool, const json& itemEntry, int quantity) {
    if (itemEntry.contains("durability")) {
        if (itemEntry["durability"].is_number_integer() || itemEntry["durability"].is_number_unsigned()) {
            const auto maxDurability = static_cast<std::int64_t>(tool.getMaxDurability());
            std::int64_t savedDurability = 0;
            if (itemEntry["durability"].is_number_unsigned()) {
                const auto unsignedDurability = itemEntry["durability"].get<std::uint64_t>();
                savedDurability = unsignedDurability > static_cast<std::uint64_t>(maxDurability)
                    ? maxDurability
                    : static_cast<std::int64_t>(unsignedDurability);
            } else {
                savedDurability = itemEntry["durability"].get<std::int64_t>();
            }
            const auto clampedDurability = std::clamp(savedDurability, std::int64_t{0}, maxDurability);
            tool.setDurability(static_cast<int>(clampedDurability));
        } else {
            std::cerr << "Warning: Invalid durability for tool '"
                      << tool.getName() << "', loading at full durability" << std::endl;
            tool.resetDurability();
        }
    } else {
        tool.resetDurability();
    }

    if (quantity > 0 && tool.isBroken()) {
        tool.resetDurability();
    }
}

bool loadMutationList(const json& mutationValues, std::vector<MutationType>& mutations) {
    mutations.clear();
    if (!mutationValues.is_array()) return false;

    for (const auto& value : mutationValues) {
        if (!value.is_number_integer()) return false;

        int mutation = value.get<int>();
        if (mutation < static_cast<int>(MutationType::WET) ||
            mutation > static_cast<int>(MutationType::CELESTIAL)) {
            return false;
        }

        mutations.push_back(static_cast<MutationType>(mutation));
    }

    return true;
}

json serializeMutationList(const std::vector<MutationType>& mutations) {
    json mutationValues = json::array();
    for (MutationType mutation : mutations) {
        mutationValues.push_back(static_cast<int>(mutation));
    }

    return mutationValues;
}

Mutation makeMutationFromType(MutationType type) {
    switch (type) {
        case MutationType::WET:
            return Mutation(MutationType::WET, 2.0f, WeatherType::RAIN);
        case MutationType::SHOCKED:
            return Mutation(MutationType::SHOCKED, 100.0f, WeatherType::THUNDER_STORM);
        case MutationType::FROZEN:
            return Mutation(MutationType::FROZEN, 5.0f, WeatherType::FROST);
        case MutationType::CELESTIAL:
            return Mutation(MutationType::CELESTIAL, 150.0f, WeatherType::METEOR_SHOWER);
    }

    return Mutation(MutationType::WET, 2.0f, WeatherType::RAIN);
}

json readExistingSaveForMerge() {
    std::ifstream inFile("save.json");
    if (!inFile.is_open()) return json::object();

    try {
        json save;
        inFile >> save;
        return save.is_object() ? save : json::object();
    } catch (const std::exception& e) {
        std::cerr << "Warning: Could not read existing save.json while saving: "
                  << e.what() << std::endl;
        return json::object();
    }
}

void restoreHarvestBasketFromSave(const json& save, std::vector<BasketEntry>* harvestBasket) {
    if (harvestBasket == nullptr ||
        !save.contains("harvestBasket") ||
        !save["harvestBasket"].is_array()) {
        return;
    }

    std::vector<BasketEntry> loadedBasket;
    for (const auto& entry : save["harvestBasket"]) {
        try {
            if (!entry.is_object() ||
                !entry.contains("cropName") ||
                !entry["cropName"].is_string() ||
                !entry.contains("price") ||
                !entry["price"].is_number() ||
                !entry.contains("mutations") ||
                !entry["mutations"].is_array()) {
                continue;
            }

            std::string cropName = entry["cropName"].get<std::string>();
            if (cropName.empty()) continue;

            double price = entry["price"].get<double>();
            std::vector<MutationType> mutations;
            if (!loadMutationList(entry["mutations"], mutations)) continue;

            loadedBasket.push_back({HarvestedItem(price, std::move(mutations)), cropName});
        } catch (const std::exception& e) {
            std::cerr << "Warning: Skipping invalid harvest basket entry: "
                      << e.what() << std::endl;
        }
    }

    harvestBasket->swap(loadedBasket);
}
}

Game::Game()
    : tickSystem_(1.0f), // 1 tick per second by default
      garden_(static_cast<int>(BOARD_COLS), static_cast<int>(BOARD_ROWS)),
      weatherSystem_(),
      player_(),
      lastSaveTimestamp_(0),
      loadedSave_(false),
      initialized_(false) {
    // Register random events
    randEventMgr_.registerEvent(std::make_unique<RaccoonEvent>(2));
    
    // Load the previous game state if it exists
    loadGame();
}

void Game::update(float deltaTime) {
    tickSystem_.update(deltaTime);
    randEventMgr_.update(deltaTime, garden_, player_);
    // In a real implementation, we would check if a tick occurred and update other systems
}

void Game::autoSave() {
    saveGame();
}

void Game::bindHarvestBasket(std::vector<BasketEntry>& harvestBasket) {
    harvestBasket_ = &harvestBasket;
    if (loadedSave_) {
        restoreHarvestBasketFromSave(readExistingSaveForMerge(), harvestBasket_);
    }
}

void Game::unbindHarvestBasket() {
    harvestBasket_ = nullptr;
}

void Game::resetStateForLoad() {
    player_ = Player();
    garden_ = Garden(static_cast<int>(BOARD_COLS), static_cast<int>(BOARD_ROWS));
    tickSystem_.reset();

    if (harvestBasket_ != nullptr) {
        harvestBasket_->clear();
    }

    loadedSave_ = false;
    initialized_ = false;
    lastSaveTimestamp_ = 0;
}

void Game::saveGame() {
    try {
        json j = readExistingSaveForMerge();
        const std::int64_t saveTimestamp = currentEpochSeconds();
        
        // Save player sheckles
        j["player"] = json::object();
        j["player"]["sheckles"] = player_.getSheckles();
        
        // Save inventory items with quantities
        json invItems = json::array();
        const auto& items = player_.getInventory().getItems();
        for (const auto& item : items) {
            std::string itemName = item->getName();
            int qty = player_.getInventory().getQuantity(itemName);
            if (qty > 0) {
                json itemJson = {
                    {"name", itemName},
                    {"quantity", qty}
                };

                if (const auto* tool = dynamic_cast<const Tool*>(item.get())) {
                    itemJson["durability"] = tool->getDurability();
                }

                invItems.push_back(std::move(itemJson));
            }
        }
        j["inventory"] = json::object();
        j["inventory"]["items"] = invItems;
        
        // Save garden plants
        json gardenPlants = json::array();
        for (int y = 0; y < garden_.getHeight(); ++y) {
            for (int x = 0; x < garden_.getWidth(); ++x) {
                const auto& cell = garden_.getCell(x, y);
                Plant* plant = cell.getPlant();
                if (plant != nullptr) {
                    json mutations = serializeMutationList(plant->getMutations());
                    gardenPlants.push_back({
                        {"x", x},
                        {"y", y},
                        {"name", plant->getName()},
                        {"stage", plant->getStage()},
                        {"maxStages", plant->getMaxStages()},
                        {"ticksElapsed", plant->getTicksElapsed()},
                        {"mutations", mutations}
                    });
                }
            }
        }
        j["garden"] = json::object();
        j["garden"]["plants"] = gardenPlants;
        
        // Save tick count
        if (!j.contains("game") || !j["game"].is_object()) {
            j["game"] = json::object();
        }
        j["game"]["tick"] = tickSystem_.getTick();
        j["game"]["initialized"] = initialized_;
        j["game"]["saveTimestamp"] = saveTimestamp;

        if (hasHarvestBasket()) {
            json basket = json::array();
            for (const auto& entry : *harvestBasket_) {
                basket.push_back({
                    {"cropName", entry.cropName_},
                    {"price", entry.item_.getPrice()},
                    {"mutations", serializeMutationList(entry.item_.getMutationList())}
                });
            }
            j["harvestBasket"] = basket;
        }
        
        // Write to file
        std::ofstream outFile("save.json");
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open save.json for writing" << std::endl;
            return;
        }
        outFile << j.dump(2);
        outFile.close();
        
        lastSaveTimestamp_ = saveTimestamp;
        loadedSave_ = true;
        std::cout << "Game saved to save.json. Timestamp: " << lastSaveTimestamp_ << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error saving game: " << e.what() << std::endl;
    }
}

void Game::loadGame() {
    json j;
    try {
        std::ifstream inFile("save.json");
        if (!inFile.is_open()) {
            // File doesn't exist, no previous save to load
            return;
        }
        
        inFile >> j;
        inFile.close();
    } catch (const std::exception& e) {
        std::cerr << "Error reading save.json: " << e.what() << std::endl;
        return;
    }

    try {
        const auto catalogue = makeShopCatalogue();
        const std::int64_t loadTimestamp = currentEpochSeconds();
        resetStateForLoad();
        lastSaveTimestamp_ = loadTimestamp;

        if (j.contains("game") && j["game"].is_object()) {
            initialized_ = j["game"].value("initialized", false);
            const auto& gameData = j["game"];
            if (gameData.contains("saveTimestamp") &&
                (gameData["saveTimestamp"].is_number_integer() || gameData["saveTimestamp"].is_number_unsigned())) {
                const std::int64_t savedTimestamp = gameData["saveTimestamp"].get<std::int64_t>();
                if (savedTimestamp > 0) {
                    lastSaveTimestamp_ = savedTimestamp;
                }
            }
        }
        
        // Restore player sheckles
        if (j.contains("player") && j["player"].contains("sheckles")) {
            float sheckles = j["player"]["sheckles"];
            player_.addSheckles(sheckles);
        }
        
        // Restore inventory items
        if (j.contains("inventory") && j["inventory"].contains("items")) {
            for (const auto& itemEntry : j["inventory"]["items"]) {
                std::string itemName = itemEntry["name"];
                int quantity = itemEntry["quantity"];
                const ShopItemDef* def = findDefByName(catalogue, itemName);
                if (!def) {
                    std::cerr << "Warning: Unknown inventory item '" << itemName << "', skipping" << std::endl;
                    continue;
                }

                auto item = makeInventoryItemFromDef(*def);
                if (!item) {
                    std::cerr << "Warning: Could not restore inventory item '"
                              << itemName << "', skipping" << std::endl;
                    continue;
                }

                if (auto* tool = dynamic_cast<Tool*>(item.get())) {
                    restoreToolDurabilityFromSave(*tool, itemEntry, quantity);
                }

                player_.getInventory().addItem(std::move(item), quantity);
            }
        }
        
        // Restore garden plants
        if (j.contains("garden") && j["garden"].contains("plants")) {
            for (const auto& plantEntry : j["garden"]["plants"]) {
                int x = plantEntry["x"];
                int y = plantEntry["y"];
                std::string cropName = plantEntry["name"];
                int stage = plantEntry.value("stage", 0);
                int ticksElapsed = plantEntry.value("ticksElapsed", 0);

                if (x < 0 || y < 0 || x >= garden_.getWidth() || y >= garden_.getHeight()) {
                    std::cerr << "Warning: Saved crop '" << cropName
                              << "' has out-of-bounds coordinates ("
                              << x << ", " << y << "), skipping" << std::endl;
                    continue;
                }

                const ShopItemDef* def = findDefByCropName(catalogue, cropName);
                if (!def) {
                    std::cerr << "Warning: Unknown crop type '" << cropName << "', skipping" << std::endl;
                    continue;
                }

                auto plant = makePlant(*def);
                if (ticksElapsed > 0) {
                    plant->grow(static_cast<std::size_t>(ticksElapsed));
                } else if (stage > 0) {
                    plant->grow(static_cast<std::size_t>(stage) * def->growTicks / std::max(def->maxStages, 1));
                }

                if (plantEntry.contains("mutations")) {
                    std::vector<MutationType> mutations;
                    if (loadMutationList(plantEntry["mutations"], mutations)) {
                        for (MutationType mutation : mutations) {
                            plant->addMutation(makeMutationFromType(mutation));
                        }
                    } else {
                        std::cerr << "Warning: Invalid mutation data for crop '"
                                  << cropName << "', loading without mutations" << std::endl;
                    }
                }

                garden_.plantCrop(x, y, std::move(plant));
            }
        }
        
        // Restore tick count
        if (j.contains("game") && j["game"].contains("tick")) {
            std::size_t savedTick = j["game"]["tick"];
            tickSystem_.reset(savedTick);
        }

        restoreHarvestBasketFromSave(j, harvestBasket_);
        loadedSave_ = true;
        processOfflineProgress();
        
        std::cout << "Game loaded from save.json" << std::endl;
        
    } catch (const std::exception& e) {
        loadedSave_ = false;
        initialized_ = false;
        std::cerr << "Error loading game: " << e.what() << std::endl;
    }
}

void Game::processOfflineProgress() {
    if (!loadedSave_ || lastSaveTimestamp_ <= 0) {
        return;
    }

    const std::int64_t now = currentEpochSeconds();
    const std::int64_t elapsedSeconds = now - lastSaveTimestamp_;
    if (elapsedSeconds <= 0) {
        lastSaveTimestamp_ = now;
        return;
    }

    const auto offlineTicks = static_cast<std::size_t>(elapsedSeconds);
    tickSystem_.fastForward(offlineTicks);

    for (int y = 0; y < garden_.getHeight(); ++y) {
        for (int x = 0; x < garden_.getWidth(); ++x) {
            if (Plant* plant = garden_.getCell(x, y).getPlant()) {
                plant->grow(offlineTicks);
            }
        }
    }

    lastSaveTimestamp_ = now;
}
