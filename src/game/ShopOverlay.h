#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

#include "../core/Game.h"
#include "../entities/HarvestedItem.h"
#include "../ui/DrawUtils.h"
#include "CropRenderer.h"
#include "ShopData.h"

struct BasketEntry;
class GameScreen;

class ShopOverlay {
public:
    ShopOverlay(sf::RenderWindow& window, sf::Font& font, Game& game,
                std::vector<BasketEntry>& harvestBasket,
                const std::vector<ShopItemDef>& catalogue,
                GameScreen& owner);
    ~ShopOverlay();

    void draw(sf::Vector2f mouse);
    void handleClick(sf::Vector2f pos);
    bool isOpen() const;
    void open(int tab = 0);
    void close();

private:
    sf::RenderWindow& window_;
    sf::Font& font_;
    Game& game_;
    std::vector<BasketEntry>& harvestBasket_;
    const std::vector<ShopItemDef>& catalogue_;
    GameScreen& owner_;

    bool shopOpen_ = false;
    int shopTab_ = 0;

    void drawSeedCard(const ShopItemDef& def, sf::FloatRect b, sf::Vector2f mouse);
    void drawToolCard(const ShopItemDef& def, sf::FloatRect b, sf::Vector2f mouse);
    void drawSellPage(sf::Vector2f mouse);

    sf::FloatRect getSeedCardRect(int i, int cols, float cardsX, float cardsY, float cardW, float cardH) const;
    sf::FloatRect getSeedButtonRect(sf::FloatRect cardRect) const;
    sf::FloatRect getToolCardRect(int i, float cardsX, float cardsY, float cardW, float cardH) const;
    sf::FloatRect getToolButtonRect(sf::FloatRect cardRect) const;
    sf::FloatRect getSellCardRect(int i, int cols, float cardsX, float gridY, float cardW, float cardH) const;
    sf::FloatRect getSellButtonRect(sf::FloatRect cardRect, float iconSz) const;

    const ShopItemDef* findItemByCrop(const std::string& cropName) const;
};
