#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include <vector>

#include "../core/Game.h"
#include "../ui/DrawUtils.h"
#include "ShopData.h"

class GameScreen;

class InventoryOverlay {
public:
    InventoryOverlay(sf::RenderWindow& window, sf::Font& font,
                     Game& game, const std::vector<ShopItemDef>& catalogue,
                     GameScreen& owner);

    void draw(sf::Vector2f mouse);
    void handleClick(sf::Vector2f pos);
    void beginDrag(sf::Vector2f pos);
    void finishDrag(sf::Vector2f pos);
    void cancelDrag();
    bool isOpen() const;
    void open();
    void close();

private:
    std::vector<std::string> inventorySlots_;
    struct DragState {
        int slot = -1;
        std::string item;

        bool isValid() const { return slot >= 0 && !item.empty(); }
    };
    std::optional<DragState> drag_;

    void syncSlots();
    int slotAt(sf::Vector2f pos) const;
    bool isMenuSlot(sf::Vector2f pos) const;
    void drawBar(sf::Vector2f mouse);
    void drawOverlay(sf::Vector2f mouse);
    void handleOverlayClick(sf::Vector2f pos);

    const ShopItemDef* findItem(const std::string& name) const;

    sf::RenderWindow& window_;
    sf::Font& font_;
    Game& game_;
    const std::vector<ShopItemDef>& catalogue_;
    GameScreen& owner_;
    bool open_ = false;
};
