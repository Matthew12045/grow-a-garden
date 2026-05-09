#pragma once

#include <string>

#include "../entities/HarvestedItem.h"

// Pairs a harvested item with the plant name it came from (for display)
struct BasketEntry {
    HarvestedItem item;
    std::string cropName;
};
