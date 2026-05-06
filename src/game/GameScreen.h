#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <array>

#include "../ui/AudioManager.h"
#include "../core/Game.h"
#include "../systems/Shop.h"
#include "../entities/HarvestedItem.h"
#include "../items/Seed.h"
#include "../entities/Carrot.h"

// ─── Layout (1920x1080) ─────────────────────────────────────────────
inline constexpr float CELL_SZ      = 144.f;
inline constexpr float BOARD_PAD    = 12.f;
inline constexpr float BOARD_W      = 5 * CELL_SZ + BOARD_PAD * 2; // Changed from 20 to 5
inline constexpr float BOARD_H      = 4 * CELL_SZ + BOARD_PAD * 2; // Changed from 20 to 4
inline constexpr float BOARD_X      = (1920.f - BOARD_W) / 2.f;
inline constexpr float BOARD_Y      = 88.f;
inline constexpr float GRID_X       = BOARD_X + BOARD_PAD;
inline constexpr float GRID_Y       = BOARD_Y + BOARD_PAD;
inline constexpr float INV_BAR_Y    = BOARD_Y + BOARD_H + 16.f;
inline constexpr float INV_BAR_H    = 72.f;
inline constexpr float INV_SLOT_SZ  = 64.f;
inline constexpr int   INV_SLOTS    = 9;

// ─── Palette ─────────────────────────────────────────────────────────
namespace Pal {
    inline const sf::Color SKY        { 108, 188, 228 };
    inline const sf::Color CLOUD      { 240, 248, 255 };
    inline const sf::Color BOARD_MID  { 158,  97,  50 };
    inline const sf::Color BOARD_DRK  {  82,  48,  18 };
    inline const sf::Color BOARD_LIT  { 200, 140,  78 };
    inline const sf::Color SOIL_MID   { 130,  82,  40 };
    inline const sf::Color SOIL_DRK   {  92,  54,  18 };
    inline const sf::Color SOIL_LIT   { 170, 120,  64 };
    inline const sf::Color FRAME_MID  { 178, 114,  54 };
    inline const sf::Color FRAME_DRK  {  92,  52,  16 };
    inline const sf::Color FRAME_LIT  { 218, 166,  94 };
    inline const sf::Color GOLD       { 255, 218,  30 };
    inline const sf::Color CREAM      { 255, 248, 220 };
    inline const sf::Color DARKTEXT   {  38,  22,   8 };
    inline const sf::Color SPROUT     {  68, 170,  68 };
    inline const sf::Color RIPE       { 255, 195,  28 };
    inline const sf::Color MUTATION   { 220,  80, 240 };
}

struct CloudDef { float x, y, scale; };

class GameScreen {
public:
    GameScreen(sf::RenderWindow& window, sf::Font& font);
    void run();

private:
    AudioManager audioManager_;
    sf::RenderWindow& window_;
    sf::Font&         font_;

    Game  game_;
    Shop  shop_;
    std::vector<HarvestedItem> harvestBasket_;

    std::string statusMsg_;
    float       statusTimer_ = 0.f;
    std::size_t lastTick_    = 0;
    std::size_t dayCount_    = 1;
    bool        fastMode_    = false;

    void handleEvent(const sf::Event& ev);
    void onMouseClick(sf::Vector2f pos);
    void update(float dt);

    void handleCellClick(sf::Vector2f pos);
    void plantSeed(int gx, int gy);
    void harvestCell(int gx, int gy);
    void buySeed();
    void sellAll();
    void setStatus(const std::string& msg, float dur = 2.5f);

    void render();
    void drawBackground();
    void drawCloud(float cx, float cy, float scale);
    void drawGardenBoard(sf::Vector2f mouse);
    void drawCell(int gx, int gy, sf::Vector2f screen, bool hovered);
    void drawPlant(Plant* plant, sf::Vector2f screen);
    void drawTopUI();
    void drawPxPanel(sf::Vector2f pos, sf::Vector2f size);
    void drawInventoryBar(sf::Vector2f mouse);
    void drawShopButton(sf::Vector2f mouse);
    void drawStatus();

    sf::Text makeText(const std::string& s, unsigned sz,
                      sf::Color col = Pal::CREAM) const;
    void centreText(sf::Text& t, sf::FloatRect area);

    void setupShop();
};