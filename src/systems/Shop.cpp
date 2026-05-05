#include "Shop.h"
#include "../core/Player.h" // We need Player full def to deduct sheckles

void Shop::addAvailableItem(std::unique_ptr<Item> item) {
    if (item) {
        availableInventory_.push_back(std::move(item));
    }
}

// In real code processPurchase/Sale would need Player header
// and Inventory integration. For now, skeleton matches UML.
bool Shop::processPurchase(Item* item, Player* player) {
    if (!item || !player) return false;

    // UML: Player::deductSheckles(amount)
    // UML: Player::buy(...) calls Shop::processPurchase(...)
    // UML implies Player manages its own Inventory updates or Shop tells Player
    // For UML fidelity we return bool. We assume Player deducts/adds items.
    return true;
}

bool Shop::processSale(std::unique_ptr<Item> item, Player* player) {
    if (!item || !player) return false;

    // Player sells item, gets sheckles
    return true;
}

const std::vector<std::unique_ptr<Item>>& Shop::getAvailableItems() const {
    return availableInventory_;
}