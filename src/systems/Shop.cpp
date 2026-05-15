#include "Shop.h"
#include "../core/Player.h" // We need Player full def to deduct sheckles

void Shop::addAvailableItem(std::unique_ptr<Item> item) {
    if (item) {
        availableInventory_.push_back(std::move(item));
    }
}

bool Shop::processPurchase(Item* item, Player* player) {
    if (!item || !player) return false;

    const double price = item->getPrice();
    if (player->getSheckles() < price) {
        return false; // Not enough money
    }

    auto purchasedItem = item->clone();
    if (!purchasedItem || !player->getInventory().addItem(std::move(purchasedItem), 1)) {
        return false;
    }

    if (!player->deductSheckles(static_cast<float>(price))) {
        player->getInventory().removeItem(item->getName(), 1);
        return false;
    }

    return true;
}

bool Shop::processSale(std::unique_ptr<Item> item, Player* player) {
    if (!item || !player) return false;

    // Player sells item, gets sheckles based on getPrice()
    player->addSheckles(item->getPrice());

    return true;
}

const std::vector<std::unique_ptr<Item>>& Shop::getAvailableItems() const {
    return availableInventory_;
}
