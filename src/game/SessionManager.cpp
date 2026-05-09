#include "SessionManager.h"

#include "../items/Seed.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <utility>

using json = nlohmann::json;

namespace {
std::vector<MutationType> loadMutationList(const json& mutationValues) {
    std::vector<MutationType> mutations;
    if (!mutationValues.is_array()) return mutations;

    for (const auto& value : mutationValues) {
        if (!value.is_number_integer()) continue;

        int mutation = value.get<int>();
        if (mutation >= static_cast<int>(MutationType::WET) &&
            mutation <= static_cast<int>(MutationType::CELESTIAL)) {
            mutations.push_back(static_cast<MutationType>(mutation));
        }
    }

    return mutations;
}
} // namespace

SessionManager::SessionManager(Game& game,
                               std::vector<BasketEntry>& harvestBasket,
                               const std::vector<ShopItemDef>& catalogue)
    : game_(game),
      harvestBasket_(harvestBasket),
      catalogue_(catalogue)
{
    game_.bindHarvestBasket(harvestBasket_);
}

SessionManager::~SessionManager() {
    game_.unbindHarvestBasket();
}

void SessionManager::initOrLoad() {
    loadHarvestBasket();
    if (!game_.hasLoadedSave() || !isInitializedSave()) {
        if (needsStarterRescue()) {
            addStartingItems();
        }
        save();
    }
}

void SessionManager::save() {
    game_.saveGame();
}

void SessionManager::addStartingItems() {
    game_.getPlayer().addSheckles(100.0f);
    auto seed = std::make_unique<Seed>(1, "Carrot Seed",
                                       "Grows into a carrot.", 10.0, "Carrot");
    game_.getPlayer().getInventory().addItem(std::move(seed), 5);
}

bool SessionManager::needsStarterRescue() const {
    if (!harvestBasket_.empty()) return false;

    std::optional<float> cheapestSeed;
    for (const auto& def : catalogue_) {
        if (def.type != ShopItemType::SEED) continue;
        if (!cheapestSeed.has_value() || def.buyPrice < *cheapestSeed) {
            cheapestSeed = def.buyPrice;
        }

        if (game_.getPlayer().getInventory().getQuantity(def.name) > 0) {
            return false;
        }
    }

    if (cheapestSeed.has_value() && game_.getPlayer().getSheckles() >= *cheapestSeed) {
        return false;
    }

    Garden& garden = game_.getGarden();
    for (int y = 0; y < garden.getHeight(); ++y) {
        for (int x = 0; x < garden.getWidth(); ++x) {
            if (garden.getCell(x, y).getPlant()) {
                return false;
            }
        }
    }

    return true;
}

bool SessionManager::isInitializedSave() const {
    try {
        std::ifstream inFile(SAVE_FILE);
        if (!inFile.is_open()) return false;

        json save;
        inFile >> save;
        return save.contains("game") &&
               save["game"].value("initialized", false);
    } catch (const std::exception&) {
        return false;
    }
}

void SessionManager::loadHarvestBasket() {
    try {
        std::ifstream inFile(SAVE_FILE);
        if (!inFile.is_open()) return;

        json save;
        inFile >> save;
        if (!save.contains("harvestBasket") || !save["harvestBasket"].is_array()) return;

        harvestBasket_.clear();
        for (const auto& entry : save["harvestBasket"]) {
            std::string cropName = entry.value("cropName", "");
            double price = entry.value("price", 0.0);
            if (cropName.empty()) continue;

            std::vector<MutationType> mutations = loadMutationList(entry.value("mutations", json::array()));
            harvestBasket_.push_back({HarvestedItem(price, std::move(mutations)), cropName});
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading harvest basket: " << e.what() << std::endl;
    }
}
