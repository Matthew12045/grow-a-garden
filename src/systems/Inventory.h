#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "../items/Item.h"

class Inventory {
private:
    std::unordered_map<std::string, int> quantities_;
    std::vector<std::unique_ptr<Item>> itemPrototypes_; // Keep prototype for iteration
    int maxSlots_;

public:
    explicit Inventory(int maxSlots);

    bool addItem(std::unique_ptr<Item> item, int qty);
    bool removeItem(const std::string& itemName, int qty);
    int getQuantity(const std::string& itemName) const;
    bool isFull() const;
    const std::vector<std::unique_ptr<Item>>& getItems() const;
};