#include "Player.h"
#include "../world/Cell.h"
#include "../systems/Shop.h"

#include <utility>

Player::Player() : sheckles_(0.0f), inventory_(20), equippedTool_(nullptr) {}

bool Player::buy(Item* item, Shop* shop) {
    if (!item || !shop) return false;
    return shop->processPurchase(item, this);
}

bool Player::sell(Item* item, Shop* shop) {
    if (!item || !shop) return false;

    const std::string& itemName = item->getName();
    if (inventory_.getQuantity(itemName) <= 0) {
        return false;
    }

    const Item* ownedPrototype = nullptr;
    for (const auto& ownedItem : inventory_.getItems()) {
        if (ownedItem && ownedItem->getName() == itemName) {
            ownedPrototype = ownedItem.get();
            break;
        }
    }

    if (!ownedPrototype) {
        return false;
    }

    auto itemForSale = ownedPrototype->clone();
    auto itemForRestore = ownedPrototype->clone();
    if (!itemForSale || !itemForRestore || !inventory_.removeItem(itemName, 1)) {
        return false;
    }

    if (!shop->processSale(std::move(itemForSale), this)) {
        inventory_.addItem(std::move(itemForRestore), 1);
        return false;
    }

    return true;
}

void Player::equipTool(std::shared_ptr<Tool> tool) {
    equippedTool_ = tool;
}

void Player::useTool(Cell& cell) {
    if (equippedTool_) {
        equippedTool_->use(cell, *this);
    }
}

void Player::addSheckles(float amount) {
    sheckles_ += amount;
}

bool Player::deductSheckles(float amount) {
    if (sheckles_ >= amount) {
        sheckles_ -= amount;
        return true;
    }
    return false;
}
