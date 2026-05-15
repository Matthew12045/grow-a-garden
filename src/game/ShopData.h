#pragma once
#include <SFML/Graphics/Color.hpp>
#include <string>
#include <vector>

enum class ShopItemType { SEED, TOOL };

struct ShopItemDef {
    std::string  name;         // inventory key   ("Carrot Seed", "Watering Can" …)
    std::string  cropName;     // plant/display name
    std::string  description;  // one-line flavour text
    float        buyPrice;     // cost in sheckles
    float        sellPrice;    // expected harvest value (0 for tools)
    int          growTicks;    // total ticks to full growth (0 for tools)
    int          maxStages;    // growth stages for plant visuals/logic
    bool         regrowsAfterHarvest;
    int          regrowStage;
    sf::Color    iconPrimary;  // main icon fill
    sf::Color    iconAccent;   // leaf / stem / accent colour
    ShopItemType type;
    int          toolBoost;    // ticks applied per tool use (0 for seeds)
};

// Returns the canonical shop catalogue: seeds first, then tools.
inline std::vector<ShopItemDef> makeShopCatalogue() {
    return {
        // name              cropName      description                          buy   sell  ticks  stages regrow regrowStage  primary            accent           type                  boost
        {"Carrot Seed",    "Carrot",    "Fast grower, reliable income",          10,   30,   50,  5,     false, 0,          {220,110, 30},  { 68,170, 68},  ShopItemType::SEED,   0},
        {"Blueberry Seed", "Blueberry", "Regrows after harvest!",                15,   45,   32,  4,     true,  1,          { 80, 80,200},  { 50,160, 80},  ShopItemType::SEED,   0},
        {"Rose Seed",      "Rose",      "Decorative and high value",             25,   80,   72,  6,     false, 0,          {220, 40, 80},  { 50,140, 50},  ShopItemType::SEED,   0},
        {"Bamboo Seed",    "Bamboo",    "Rapid grower, regrows after harvest",   12,   35,   45,  3,     true,  0,          { 60,180, 60},  { 40,140, 40},  ShopItemType::SEED,   0},
        {"Corn Seed",      "Corn",      "Sunny staple crop",                     20,   60,   70,  5,     false, 0,          {240,200, 30},  { 70,170, 50},  ShopItemType::SEED,   0},
        {"Tomato Seed",    "Tomato",    "Juicy, regrows after harvest",          22,   65,   60,  6,     true,  2,          {220, 50, 40},  { 50,140, 50},  ShopItemType::SEED,   0},
        {"Apple Seed",     "Apple",     "Slow growing, very valuable",           50,  150,  160,  8,     false, 0,          {200, 55, 55},  { 60,160, 60},  ShopItemType::SEED,   0},
        {"Cactus Seed",    "Cactus",    "Desert hardy, no water needed",         30,   90,  100,  4,     false, 0,          { 80,160, 60},  {100,140, 60},  ShopItemType::SEED,   0},
        {"Coconut Seed",   "Coconut",   "Tropical treasure, top earner",         75,  200,  175,  7,     false, 0,          {200,160, 80},  { 60,120, 40},  ShopItemType::SEED,   0},
        {"Beanstalk Seed", "Beanstalk", "Legendary climber, huge payout",        60,  220,  160,  8,     false, 0,          { 50,200, 80},  { 30,130, 55},  ShopItemType::SEED,   0},
        {"Cacao Seed",     "Cacao",     "Rare chocolate tree",                   45,  130,  108,  6,     false, 0,          {120, 70, 30},  { 80,150, 55},  ShopItemType::SEED,   0},
        // ── Tools ──────────────────────────────────────────────────────────────────────────────────────────────────────────
        {"Watering Can",  "Watering Can","Click a plant to boost +5 ticks",     35,    0,    0,  0,     false, 0,          { 80,145,225},  { 50,100,180},  ShopItemType::TOOL,   5},
        {"Fertilizer",    "Fertilizer", "Click a plant to boost +20 ticks",     50,    0,    0,  0,     false, 0,          {160,120, 40},  {100,175, 60},  ShopItemType::TOOL,  20},
    };
}
