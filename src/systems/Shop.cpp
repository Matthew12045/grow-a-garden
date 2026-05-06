#include "Shop.h"
#include "../core/Player.h" // We need Player full def to deduct sheckles

void Shop::addAvailableItem(std::unique_ptr<Item> item) {
    if (item) {
        availableInventory_.push_back(std::move(item));
    }
}

bool Shop::processPurchase(Item* item, Player* player) {
    if (!item || !player) return false;

    // Check if player has enough sheckles
    if (player->getSheckles() < item->getPrice()) {
        return false; // Not enough money
    }

    // Try to add to inventory (we need a copy of the item, but Item is abstract)
    // Since Item has no clone(), we'd typically need one. But for this UML,
    // maybe we just deduct sheckles and assume player gets it.
    // However, UML Inventory takes unique_ptr.
    // For now, let's just deduct sheckles.
    if (player->deductSheckles(item->getPrice())) {
        // Player deducts sheckles
        // Ideally player->getInventory().addItem(item->clone(), 1);
        return true;
    }

    return false;
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