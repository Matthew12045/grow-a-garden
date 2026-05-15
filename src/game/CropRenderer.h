#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

#include "../entities/Plant.h"
#include "../ui/DrawUtils.h"
#include "ShopData.h"

namespace CropRenderer {
void drawCropIcon(sf::RenderWindow& window, sf::Font& font, const ShopItemDef& def,
                  sf::Vector2f centre, float sz);
void drawPlant(sf::RenderWindow& window, sf::Font& font, Plant* plant, sf::Vector2f screen,
               sf::Vector2f cellSize, const std::vector<ShopItemDef>& catalogue);
void drawCloud(sf::RenderWindow& window, float cx, float cy, float scale);
}
