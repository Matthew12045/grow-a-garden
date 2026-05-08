#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

#include "../ui/AudioManager.h"
#include "../core/Game.h"
#include "../systems/Shop.h"
#include "../entities/HarvestedItem.h"
#include "../items/Seed.h"
#include "ShopData.h"
#include "PlantFactory.h"

// ─── Layout constants (1920×1080) ────────────────────────────────────
inline constexpr float CELL_SZ    = 144.f;
inline constexpr float BOARD_PAD  = 12.f;
inline constexpr float BOARD_COLS = 5.f;
inline constexpr float BOARD_ROWS = 4.f;
inline constexpr float BOARD_W    = BOARD_COLS * CELL_SZ + BOARD_PAD * 2;
inline constexpr float BOARD_H    = BOARD_ROWS * CELL_SZ + BOARD_PAD * 2;
inline constexpr float BOARD_X    = (1920.f - BOARD_W) / 2.f;
inline constexpr float BOARD_Y    = 88.f;
inline constexpr float GRID_X     = BOARD_X + BOARD_PAD;
inline constexpr float GRID_Y     = BOARD_Y + BOARD_PAD;
inline constexpr float INV_BAR_Y  = BOARD_Y + BOARD_H + 18.f;
inline constexpr float INV_BAR_H  = 76.f;

// Shop overlay
inline constexpr float SHOP_W     = 1460.f;
inline constexpr float SHOP_H     = 780.f;
inline constexpr float SHOP_X     = (1920.f - SHOP_W) / 2.f;
inline constexpr float SHOP_Y     = (1080.f - SHOP_H) / 2.f;

// ─── Colour palette ──────────────────────────────────────────────────
namespace Pal {
    inline const sf::Color SKY       { 108, 188, 228 };
    inline const sf::Color CLOUD     { 240, 248, 255 };
    inline const sf::Color BOARD_MID { 158,  97,  50 };
    inline const sf::Color BOARD_DRK {  82,  48,  18 };
    inline const sf::Color BOARD_LIT { 200, 140,  78 };
    inline const sf::Color SOIL_MID  { 130,  82,  40 };
    inline const sf::Color SOIL_DRK  {  92,  54,  18 };
    inline const sf::Color SOIL_LIT  { 170, 120,  64 };
    inline const sf::Color FRAME_MID { 155, 100,  48 };
    inline const sf::Color FRAME_DRK {  88,  50,  15 };
    inline const sf::Color FRAME_LIT { 215, 162,  90 };
    inline const sf::Color GOLD      { 255, 218,  30 };
    inline const sf::Color CREAM     { 255, 248, 220 };
    inline const sf::Color DARKTEXT  {  38,  22,   8 };
    inline const sf::Color SPROUT    {  68, 170,  68 };
    inline const sf::Color MUTATION  { 220,  80, 240 };
    // Shop
    inline const sf::Color SHOP_BG   {  48,  30,  12 };
    inline const sf::Color CARD_BG   { 110,  68,  32 };
    inline const sf::Color CARD_HOV  { 135,  88,  45 };
    inline const sf::Color BTN_BUY   {  48, 140,  55 };
    inline const sf::Color BTN_BHOV  {  65, 175,  72 };
    inline const sf::Color TAB_ACT   { 200, 140,  55 };
    inline const sf::Color TAB_INACT { 100,  62,  25 };
}

struct CloudDef { float x, y, scale; };

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
    void useToolOnCell(int gx, int gy);
    void setStatus(const std::string& msg, float dur = 2.5f);

    void render();
    void drawBackground();
    void drawCloud(float cx, float cy, float scale);
    void drawGardenBoard(sf::Vector2f mouse);
    void drawCell(int gx, int gy, sf::Vector2f screen, bool hovered);
    void drawPlant(Plant* plant, sf::Vector2f screen);
    void drawTopUI();
    void drawInventoryBar(sf::Vector2f mouse);
    void drawShopTabButton(sf::Vector2f mouse);
    void drawStatus();

    void drawShopOverlay(sf::Vector2f mouse);
    void drawSeedCard(const ShopItemDef& def, sf::FloatRect b, sf::Vector2f mouse);
    void drawToolCard(const ShopItemDef& def, sf::FloatRect b, sf::Vector2f mouse);
    void drawSellPage(sf::Vector2f mouse);
    void drawCropIcon(const ShopItemDef& def, sf::Vector2f centre, float sz);
    void onShopClick(sf::Vector2f pos);

    void drawPxPanel(sf::Vector2f pos, sf::Vector2f sz,
                     sf::Color mid  = Pal::FRAME_MID,
                     sf::Color dark = Pal::FRAME_DRK,
                     sf::Color lite = Pal::FRAME_LIT);
    void drawPxButton(sf::FloatRect b, const std::string& label,
                      sf::Color col, sf::Color hover,
                      sf::Vector2f mouse, unsigned fontSize = 20);
    sf::Text makeText(const std::string& s, unsigned sz,
                      sf::Color col = Pal::CREAM) const;
    void centreText(sf::Text& t, sf::FloatRect area);

    const ShopItemDef* findItem(const std::string& name) const;
    const ShopItemDef* findItemByCrop(const std::string& cropName) const;
};
