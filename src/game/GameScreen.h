#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

#include "../ui/AudioManager.h"
#include "../ui/DrawUtils.h"
#include "../core/Game.h"
#include "../systems/Shop.h"
#include "../entities/HarvestedItem.h"
#include "../items/Seed.h"
#include "ShopData.h"
#include "PlantFactory.h"

// Pairs a harvested item with the plant name it came from (for display)
struct BasketEntry {
    HarvestedItem item;
    std::string   cropName;
};

// ─────────────────────────────────────────────────────────────────────
class GameScreen {
public:
    GameScreen(sf::RenderWindow& window, sf::Font& font);
    void run();

private:
    AudioManager      audioManager_;
    sf::RenderWindow& window_;
    sf::Font&         font_;

    Game  game_;
    Shop  shop_;
    std::vector<BasketEntry>  harvestBasket_;
    std::vector<ShopItemDef>  catalogue_;

    std::string statusMsg_;
    float       statusTimer_ = 0.f;
    std::size_t lastTick_    = 0;
    std::size_t dayCount_    = 1;
    bool        fastMode_    = false;

    bool shopOpen_ = false;
    int  shopTab_  = 0;

    std::string selectedSeed_ = "Carrot Seed";
    std::string equippedTool_ = "";

    void setupShop();

    void handleEvent(const sf::Event& ev);
    void onMouseClick(sf::Vector2f pos);
    void update(float dt);

    void handleCellClick(sf::Vector2f pos);
    void plantSeed(int gx, int gy);
    void harvestCell(int gx, int gy);
    void buyItem(const ShopItemDef& def);
    void sellAll();
    void sellOne(int index);
    void sellGroup(const std::vector<int>& indices);
    void useToolOnCell(int gx, int gy);
    void setStatus(const std::string& msg, float dur = 2.5f);

    void render();
    void drawBackground();
    void drawGardenBoard(sf::Vector2f mouse);
    void drawCell(int gx, int gy, sf::Vector2f screen, bool hovered);
    void drawTopUI();
    void drawInventoryBar(sf::Vector2f mouse);
    void drawShopTabButton(sf::Vector2f mouse);
    void drawStatus();

    void drawShopOverlay(sf::Vector2f mouse);
    void drawSeedCard(const ShopItemDef& def, sf::FloatRect b, sf::Vector2f mouse);
    void drawToolCard(const ShopItemDef& def, sf::FloatRect b, sf::Vector2f mouse);
    void drawSellPage(sf::Vector2f mouse);
    void onShopClick(sf::Vector2f pos);

    sf::FloatRect getSeedCardRect(int i, int cols, float cardsX, float cardsY, float cardW, float cardH) const;
    sf::FloatRect getSeedButtonRect(sf::FloatRect cardRect) const;
    sf::FloatRect getToolCardRect(int i, float cardsX, float cardsY, float cardW, float cardH) const;
    sf::FloatRect getToolButtonRect(sf::FloatRect cardRect) const;
    sf::FloatRect getSellCardRect(int i, int cols, float cardsX, float gridY, float cardW, float cardH) const;
    sf::FloatRect getSellButtonRect(sf::FloatRect cardRect, float iconSz) const;

    const ShopItemDef* findItem(const std::string& name) const;
    const ShopItemDef* findItemByCrop(const std::string& cropName) const;
};
