#pragma once
#include <memory>
#include <string>
#include "ShopData.h"
#include "../entities/Plant.h"

// Creates Plant from shop catalogue entry.
std::unique_ptr<Plant> makePlant(const ShopItemDef& def);
