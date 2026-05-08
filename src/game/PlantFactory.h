#pragma once
#include <memory>
#include <string>
#include "../entities/Plant.h"

// Creates the correct Plant subclass for a given seed name.
std::unique_ptr<Plant> makePlant(const std::string& seedName);
