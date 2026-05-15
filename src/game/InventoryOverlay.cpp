#include "InventoryOverlay.h"

#include "CropRenderer.h"
#include "GameScreen.h"

#include <algorithm>

namespace {
constexpr int   HOTBAR_ITEM_SLOTS = 9;
constexpr int   HOTBAR_SLOTS      = HOTBAR_ITEM_SLOTS + 1;
constexpr float HOTBAR_SLOT       = 64.f;
constexpr float HOTBAR_GAP        = 4.f;
constexpr float HOTBAR_PAD        = 8.f;

constexpr int   INV_COLS   = 9;
constexpr int   INV_ROWS   = 3;
constexpr int   INV_SLOTS  = INV_COLS * INV_ROWS;
constexpr int   INV_TOTAL_SLOTS = HOTBAR_ITEM_SLOTS + INV_SLOTS;
constexpr float INV_SLOT   = HOTBAR_SLOT;
constexpr float INV_GAP    = HOTBAR_GAP;
constexpr float INV_PAD    = 24.f;
constexpr float INV_TITLE  = 52.f;
constexpr float INV_HOTBAR_GAP = 18.f;

float inventoryGridW() {
    return INV_COLS * INV_SLOT + (INV_COLS - 1) * INV_GAP;
}

float inventoryGridH() {
    return INV_ROWS * INV_SLOT + (INV_ROWS - 1) * INV_GAP;
}

float inventoryPanelW() {
    return inventoryGridW() + INV_PAD * 2.f;
}

sf::FloatRect hotbarSlotRect(int index) {
    float totalW = HOTBAR_SLOTS * INV_SLOT + (HOTBAR_SLOTS - 1) * INV_GAP;
    float panelW = totalW + HOTBAR_PAD * 2.f;
    float panelX = BOARD_X + (BOARD_W - panelW) / 2.f;
    float startX = panelX + HOTBAR_PAD;
    float startY = INV_BAR_Y + (INV_BAR_H - INV_SLOT) / 2.f;
    float sx = startX + index * (INV_SLOT + INV_GAP);
    return {{sx, startY}, {INV_SLOT, INV_SLOT}};
}

sf::FloatRect hotbarPanelRect() {
    float totalW = HOTBAR_SLOTS * INV_SLOT + (HOTBAR_SLOTS - 1) * INV_GAP;
    float panelW = totalW + HOTBAR_PAD * 2.f;
    float panelX = BOARD_X + (BOARD_W - panelW) / 2.f;
    return {{panelX, INV_BAR_Y}, {panelW, INV_BAR_H}};
}

sf::FloatRect inventoryPanelRect();

sf::FloatRect inventorySlotRect(int index) {
    sf::FloatRect panel = inventoryPanelRect();
    float panelX = panel.position.x;
    float panelY = panel.position.y;
    float startX = panelX + INV_PAD;
    float startY = panelY + INV_TITLE + INV_PAD;
    int col = index % INV_COLS;
    int row = index / INV_COLS;
    float sx = startX + col * (INV_SLOT + INV_GAP);
    float sy = startY + row * (INV_SLOT + INV_GAP);
    return {{sx, sy}, {INV_SLOT, INV_SLOT}};
}

sf::FloatRect inventoryPanelRect() {
    float panelW = inventoryPanelW();
    float panelH = INV_TITLE + inventoryGridH() + INV_PAD * 2.f;
    float panelX = hotbarSlotRect(0).position.x - INV_PAD;
    float panelY = INV_BAR_Y - INV_HOTBAR_GAP - panelH;
    return {{panelX, panelY}, {panelW, panelH}};
}
} // namespace

InventoryOverlay::InventoryOverlay(sf::RenderWindow& window, sf::Font& font,
                                   Game& game,
                                   const std::vector<ShopItemDef>& catalogue,
                                   GameScreen& owner)
    : window_(window),
      font_(font),
      game_(game),
      catalogue_(catalogue),
      owner_(owner)
{
}

bool InventoryOverlay::isOpen() const {
    return open_;
}

void InventoryOverlay::open() {
    open_ = true;
}

void InventoryOverlay::close() {
    open_ = false;
    cancelDrag();
}

void InventoryOverlay::draw(sf::Vector2f mouse) {
    syncSlots();

    if (open_) {
        drawOverlay(mouse);
    }
    drawBar(mouse);
}

void InventoryOverlay::handleClick(sf::Vector2f pos) {
    syncSlots();

    if (isMenuSlot(pos)) {
        if (open_) close();
        else open();
        return;
    }

    if (open_) {
        handleOverlayClick(pos);
        return;
    }

    int slot = slotAt(pos);
    if (slot >= 0 && slot < static_cast<int>(inventorySlots_.size()) && !inventorySlots_[slot].empty()) {
        owner_.selectInventoryItem(inventorySlots_[slot]);
    }
}

void InventoryOverlay::beginDrag(sf::Vector2f pos) {
    syncSlots();
    int slot = slotAt(pos);
    if (slot < 0 || slot >= static_cast<int>(inventorySlots_.size()) || inventorySlots_[slot].empty()) {
        cancelDrag();
        return;
    }

    drag_ = DragState{slot, inventorySlots_[slot]};
}

void InventoryOverlay::finishDrag(sf::Vector2f pos) {
    if (!drag_) {
        handleClick(pos);
        return;
    }

    DragState drag = *drag_;
    if (!drag.isValid()) {
        cancelDrag();
        handleClick(pos);
        return;
    }

    int target = slotAt(pos);
    if (target >= 0 && target < static_cast<int>(inventorySlots_.size())) {
        if (target == drag.slot) {
            owner_.selectInventoryItem(drag.item);
        } else {
            std::swap(inventorySlots_[drag.slot], inventorySlots_[target]);
        }
    }

    cancelDrag();
}

void InventoryOverlay::cancelDrag() {
    drag_.reset();
}

void InventoryOverlay::drawBar(sf::Vector2f mouse) {
    sf::FloatRect panel = hotbarPanelRect();
    DrawUtils::drawPxPanel(window_, font_, panel.position, panel.size);

    for (int i = 0; i < HOTBAR_SLOTS; ++i) {
        sf::FloatRect slot = hotbarSlotRect(i);
        float sx  = slot.position.x;
        float sy  = slot.position.y;
        float slotSz = slot.size.x;
        bool  hov = slot.contains(mouse);
        bool isMenuSlot = (i == HOTBAR_ITEM_SLOTS);
        bool hasItem = !isMenuSlot && i < static_cast<int>(inventorySlots_.size()) &&
                       !inventorySlots_[i].empty();
        std::string name = hasItem ? inventorySlots_[i] : "";
        bool sel = hasItem && (name == owner_.selectedSeed_ || name == owner_.equippedTool_);

        sf::RectangleShape shadow({slotSz, slotSz});
        shadow.setPosition({sx, sy});
        shadow.setFillColor(Pal::SOIL_DRK);
        window_.draw(shadow);

        sf::Color face = hov ? sf::Color{160,112,58} : Pal::SOIL_MID;
        sf::RectangleShape fc({slotSz-3.f, slotSz-3.f});
        fc.setPosition({sx, sy});
        fc.setFillColor(face);
        window_.draw(fc);

        if (hasItem) {
            const auto* def = findItem(name);
            int qty = game_.getPlayer().getInventory().getQuantity(name);
            if (def) CropRenderer::drawCropIcon(window_, font_, *def, {sx+slotSz/2.f, sy+slotSz*0.46f}, slotSz*0.72f);
            auto cnt = DrawUtils::makeText(font_, std::to_string(qty), 12, Pal::GOLD);
            cnt.setPosition({sx+slotSz-15.f, sy+slotSz-17.f});
            window_.draw(cnt);
        } else if (isMenuSlot) {
            auto more = DrawUtils::makeText(font_, "...", 26, Pal::CREAM);
            DrawUtils::centreText(more, sf::FloatRect{{sx, sy}, {slotSz, slotSz}});
            window_.draw(more);
        }

        if (sel || hov) {
            sf::RectangleShape outline({slotSz-3.f, slotSz-3.f});
            outline.setPosition({sx, sy});
            outline.setFillColor(sf::Color::Transparent);
            outline.setOutlineThickness(3.f);
            outline.setOutlineColor(sel ? Pal::GOLD : sf::Color{200,200,200,140});
            window_.draw(outline);
        }
    }

    if (open_ && drag_ && drag_->isValid()) {
        sf::Vector2f mousePos = window_.mapPixelToCoords(sf::Mouse::getPosition(window_));
        const auto* def = findItem(drag_->item);
        sf::RectangleShape ghost({HOTBAR_SLOT, HOTBAR_SLOT});
        ghost.setOrigin({HOTBAR_SLOT / 2.f, HOTBAR_SLOT / 2.f});
        ghost.setPosition(mousePos);
        ghost.setFillColor({130, 82, 40, 210});
        ghost.setOutlineThickness(3.f);
        ghost.setOutlineColor(Pal::GOLD);
        window_.draw(ghost);

        if (def) CropRenderer::drawCropIcon(window_, font_, *def, mousePos, HOTBAR_SLOT * 0.72f);
        int qty = game_.getPlayer().getInventory().getQuantity(drag_->item);
        auto cnt = DrawUtils::makeText(font_, std::to_string(qty), 13, Pal::GOLD);
        cnt.setPosition({mousePos.x + HOTBAR_SLOT / 2.f - 18.f, mousePos.y + HOTBAR_SLOT / 2.f - 20.f});
        window_.draw(cnt);
    }
}

void InventoryOverlay::drawOverlay(sf::Vector2f mouse) {
    sf::FloatRect panel = inventoryPanelRect();
    float panelX = panel.position.x;
    float panelY = panel.position.y;
    float panelW = panel.size.x;
    float panelH = panel.size.y;

    sf::RectangleShape dim({1920.f, 1080.f});
    dim.setFillColor({0, 0, 0, 150});
    window_.draw(dim);

    DrawUtils::drawPxPanel(window_, font_, {panelX, panelY}, {panelW, panelH},
                           Pal::SHOP_BG, {25,14,4}, {75,48,18});

    auto title = DrawUtils::makeText(font_, "INVENTORY", 28, Pal::GOLD);
    auto titleBounds = title.getLocalBounds();
    title.setPosition({panelX + (panelW - titleBounds.size.x) / 2.f, panelY + 14.f});
    window_.draw(title);

    for (int i = 0; i < INV_SLOTS; ++i) {
        sf::FloatRect slot = inventorySlotRect(i);
        int itemSlot = HOTBAR_ITEM_SLOTS + i;
        float sx = slot.position.x;
        float sy = slot.position.y;
        bool hov = slot.contains(mouse);
        bool isDraggingThis = drag_ && drag_->slot == itemSlot;
        bool hasItem = itemSlot < static_cast<int>(inventorySlots_.size()) &&
                       !inventorySlots_[itemSlot].empty() && !isDraggingThis;
        std::string name = hasItem ? inventorySlots_[itemSlot] : "";
        bool sel = hasItem && (name == owner_.selectedSeed_ || name == owner_.equippedTool_);

        sf::RectangleShape shadow({INV_SLOT, INV_SLOT});
        shadow.setPosition({sx, sy});
        shadow.setFillColor(Pal::SOIL_DRK);
        window_.draw(shadow);

        sf::RectangleShape face({INV_SLOT-3.f, INV_SLOT-3.f});
        face.setPosition({sx, sy});
        face.setFillColor(hov ? sf::Color{160,112,58} : Pal::SOIL_MID);
        window_.draw(face);

        if (hasItem) {
            const auto* def = findItem(name);
            int qty = game_.getPlayer().getInventory().getQuantity(name);
            if (def) CropRenderer::drawCropIcon(window_, font_, *def, {sx+INV_SLOT/2.f, sy+INV_SLOT*0.46f}, INV_SLOT*0.72f);
            auto cnt = DrawUtils::makeText(font_, std::to_string(qty), 13, Pal::GOLD);
            cnt.setPosition({sx+INV_SLOT-18.f, sy+INV_SLOT-20.f});
            window_.draw(cnt);
        }

        if (sel || hov) {
            sf::RectangleShape outline({INV_SLOT-3.f, INV_SLOT-3.f});
            outline.setPosition({sx, sy});
            outline.setFillColor(sf::Color::Transparent);
            outline.setOutlineThickness(3.f);
            outline.setOutlineColor(sel ? Pal::GOLD : sf::Color{200,200,200,140});
            window_.draw(outline);
        }
    }
}

void InventoryOverlay::handleOverlayClick(sf::Vector2f pos) {
    syncSlots();
    int slot = slotAt(pos);
    if (slot >= 0 && slot < static_cast<int>(inventorySlots_.size()) && !inventorySlots_[slot].empty()) {
        owner_.selectInventoryItem(inventorySlots_[slot]);
    }
}

void InventoryOverlay::syncSlots() {
    if (inventorySlots_.size() != INV_TOTAL_SLOTS) inventorySlots_.resize(INV_TOTAL_SLOTS);

    auto& inv = game_.getPlayer().getInventory();
    for (auto& slot : inventorySlots_) {
        if (!slot.empty() && inv.getQuantity(slot) <= 0) slot.clear();
    }

    const auto& items = inv.getItems();
    for (const auto& item : items) {
        if (!item) continue;
        std::string name = item->getName();
        if (inv.getQuantity(name) <= 0) continue;
        auto present = std::find(inventorySlots_.begin(), inventorySlots_.end(), name);
        if (present != inventorySlots_.end()) continue;
        auto empty = std::find(inventorySlots_.begin(), inventorySlots_.end(), "");
        if (empty != inventorySlots_.end()) *empty = name;
    }
}

int InventoryOverlay::slotAt(sf::Vector2f pos) const {
    for (int i = 0; i < HOTBAR_ITEM_SLOTS; ++i) {
        if (hotbarSlotRect(i).contains(pos)) return i;
    }

    if (!open_) return -1;

    for (int i = 0; i < INV_SLOTS; ++i) {
        if (inventorySlotRect(i).contains(pos)) return HOTBAR_ITEM_SLOTS + i;
    }
    return -1;
}

bool InventoryOverlay::isMenuSlot(sf::Vector2f pos) const {
    return hotbarSlotRect(HOTBAR_ITEM_SLOTS).contains(pos);
}

const ShopItemDef* InventoryOverlay::findItem(const std::string& name) const {
    for (const auto& def : catalogue_) {
        if (def.name == name) return &def;
    }
    return nullptr;
}
