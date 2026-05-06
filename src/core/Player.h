#pragma once

#include <memory>
#include "../systems/Inventory.h"
#include "../items/Tool.h"

class Shop;
class Item;
class Cell;

class Player {
private:
    float sheckles_;
    Inventory inventory_;
    std::shared_ptr<Tool> equippedTool_;

public:
    Player();

    bool buy(Item* item, Shop* shop);
    bool sell(Item* item, Shop* shop);

    void equipTool(std::shared_ptr<Tool> tool);
    void useTool(Cell& cell);

    void addSheckles(float amount);
    bool deductSheckles(float amount);

    float getSheckles() const { return sheckles_; }
    Inventory& getInventory() { return inventory_; }
};