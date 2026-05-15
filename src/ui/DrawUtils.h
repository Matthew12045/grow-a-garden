#pragma once

#include <SFML/Graphics.hpp>
#include <string>

// Layout constants (1920x1080)
inline constexpr float CELL_W     = 274.f;
inline constexpr float CELL_H     = 145.f;
inline constexpr float CELL_SZ    = CELL_H;
inline constexpr float BOARD_PAD  = 0.f;
inline constexpr float BOARD_COLS = 5.f;
inline constexpr float BOARD_ROWS = 4.f;
inline constexpr float BOARD_X    = 278.f;
inline constexpr float BOARD_Y    = 161.f;
inline constexpr float BOARD_W    = 1426.f;
inline constexpr float BOARD_H    = 629.f;
inline constexpr float GRID_X     = 320.f;
inline constexpr float GRID_Y     = 195.f;
inline constexpr float GRID_W     = BOARD_COLS * CELL_W;
inline constexpr float GRID_H     = BOARD_ROWS * CELL_H;
inline constexpr float INV_BAR_Y  = BOARD_Y + BOARD_H + 18.f;
inline constexpr float INV_BAR_H  = 76.f;

// Shop overlay
inline constexpr float SHOP_W     = 1460.f;
inline constexpr float SHOP_H     = 780.f;
inline constexpr float SHOP_X     = (1920.f - SHOP_W) / 2.f;
inline constexpr float SHOP_Y     = (1080.f - SHOP_H) / 2.f;

// Colour palette
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

namespace DrawUtils {
void drawPxPanel(sf::RenderWindow& window, sf::Font& font, sf::Vector2f pos, sf::Vector2f sz,
                 sf::Color mid = Pal::FRAME_MID,
                 sf::Color dark = Pal::FRAME_DRK,
                 sf::Color lite = Pal::FRAME_LIT);
void drawPxButton(sf::RenderWindow& window, sf::Font& font, sf::FloatRect b, const std::string& label,
                  sf::Color col, sf::Color hover,
                  sf::Vector2f mouse, unsigned fontSize = 20);
sf::Text makeText(const sf::Font& font, const std::string& s, unsigned sz,
                  sf::Color col = Pal::CREAM);
void centreText(sf::Text& t, sf::FloatRect area);
}
