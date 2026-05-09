#include "SessionManager.h"

#include "../items/Seed.h"

#include <memory>
#include <optional>

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
    if (!game_.hasLoadedSave() || !game_.isInitialized()) {
        if (needsStarterRescue()) {
            addStartingItems();
        }
        game_.setInitialized(true);
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
