#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <vector>

#include "../ui/AudioManager.h"
#include "../ui/DrawUtils.h"
#include "../core/Game.h"
#include "../systems/Shop.h"
#include "../items/Seed.h"
#include "HarvestBasket.h"
#include "ShopData.h"
#include "PlantFactory.h"
#include "InventoryOverlay.h"
#include "SessionManager.h"
#include "ShopOverlay.h"

// ─────────────────────────────────────────────────────────────────────
class GameScreen {
public:
    GameScreen(sf::RenderWindow& window, sf::Font& font);
    void run();
    void buyItem(const ShopItemDef& def);
    void sellAll();
    void sellOne(int index);
    void sellGroup(const std::vector<int>& indices);
    void selectInventoryItem(const std::string& name);

private:
    friend class InventoryOverlay;

    AudioManager      audioManager_;
    sf::RenderWindow& window_;
    sf::Font&         font_;

    Game  game_;
    Shop  shop_;
    std::vector<BasketEntry>  harvestBasket_;
    std::vector<ShopItemDef>  catalogue_;
    SessionManager session_;
    ShopOverlay shopOverlay_;
    InventoryOverlay inventoryOverlay_;
    sf::Texture gardenBoardTexture_;
    std::unique_ptr<sf::Sprite> gardenBoardSprite_;

    std::string statusMsg_;
    float       statusTimer_ = 0.f;
    std::size_t lastTick_    = 0;
    std::size_t dayCount_    = 1;
    bool        fastMode_    = false;

    std::string selectedSeed_ = "Carrot Seed";
    std::string equippedTool_ = "";

    void setupShop();

    void handleEvent(const sf::Event& ev);
    void onMouseClick(sf::Vector2f pos);
    void update(float dt);

    void handleCellClick(sf::Vector2f pos);
    void plantSeed(int gx, int gy);
    void harvestCell(int gx, int gy);
    void useToolOnCell(int gx, int gy);
    void setStatus(const std::string& msg, float dur = 2.5f);

    void render();
    void drawBackground();
    void drawGardenBoard(sf::Vector2f mouse);
    void drawCell(int gx, int gy, sf::Vector2f screen, bool hovered);
    void drawTopUI();
    void drawShopTabButton(sf::Vector2f mouse);
    void drawStatus();

    const ShopItemDef* findItem(const std::string& name) const;
};
