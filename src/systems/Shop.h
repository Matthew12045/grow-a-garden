#pragma once

#include <vector>
#include <memory>
#include "../items/Item.h"

class Player; // Forward declaration

class Shop {
private:
    std::vector<std::unique_ptr<Item>> availableInventory_;

public:
    Shop() = default;

    void addAvailableItem(std::unique_ptr<Item> item);

    bool processPurchase(Item* item, Player* player);
    bool processSale(std::unique_ptr<Item> item, Player* player);

    const std::vector<std::unique_ptr<Item>>& getAvailableItems() const;
};