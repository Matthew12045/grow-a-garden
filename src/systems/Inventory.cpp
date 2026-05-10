#include "Inventory.h"
#include <algorithm>

Inventory::Inventory(int maxSlots) : maxSlots_(maxSlots) {}

bool Inventory::addItem(std::unique_ptr<Item> item, int qty) {
    if (!item || qty <= 0) return false;

    std::string name = item->getName();

    // Check if new slot needed
    if (quantities_.find(name) == quantities_.end()) {
        if (isFull()) return false;
        itemPrototypes_.push_back(std::move(item));
        quantities_[name] = 0;
    }

    quantities_[name] += qty;
    return true;
}

bool Inventory::removeItem(const std::string& itemName, int qty) {
    if (qty <= 0) return false;

    auto it = quantities_.find(itemName);
    if (it == quantities_.end() || it->second < qty) {
        return false;
    }

    it->second -= qty;

    if (it->second == 0) {
        quantities_.erase(it);

        // Remove prototype
        itemPrototypes_.erase(
            std::remove_if(itemPrototypes_.begin(), itemPrototypes_.end(),
                [&itemName](const std::unique_ptr<Item>& p) { return p->getName() == itemName; }),
            itemPrototypes_.end()
        );
    }

    return true;
}

int Inventory::getQuantity(const std::string& itemName) const {
    auto it = quantities_.find(itemName);
    if (it != quantities_.end()) {
        return it->second;
    }
    return 0;
}

Item* Inventory::getItemPrototype(const std::string& itemName) {
    for (const auto& item : itemPrototypes_) {
        if (item && item->getName() == itemName) {
            return item.get();
        }
    }

    return nullptr;
}

const Item* Inventory::getItemPrototype(const std::string& itemName) const {
    for (const auto& item : itemPrototypes_) {
        if (item && item->getName() == itemName) {
            return item.get();
        }
    }

    return nullptr;
}

bool Inventory::isFull() const {
    return quantities_.size() >= static_cast<size_t>(maxSlots_);
}

const std::vector<std::unique_ptr<Item>>& Inventory::getItems() const {
    return itemPrototypes_;
}
