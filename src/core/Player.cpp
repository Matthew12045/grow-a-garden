#include "Player.h"
#include "../world/Cell.h"
#include "../systems/Shop.h"

Player::Player() : sheckles_(0.0f), inventory_(20), equippedTool_(nullptr) {}

bool Player::buy(Item* item, Shop* shop) {
    if (!item || !shop) return false;
    return shop->processPurchase(item, this);
}

bool Player::sell(Item* item, Shop* shop) {
    if (!item || !shop) return false;
    // Shop processes sale, but requires unique_ptr.
    // For now, UML indicates Player passes item to shop, but Item might be in Inventory
    // We would need to extract it from inventory_ first.
    // Wait for the UML design, passing raw pointer to shop implies shop does not take ownership
    // Oh UML processSale takes unique_ptr<Item>
    return false; // Shop::processSale(std::move(item), this);
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