#include "GameScreen.h"
#include "CropRenderer.h"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace {
std::string makeMutationKey(const std::vector<MutationType>& mutations) {
    std::vector<int> values;
    values.reserve(mutations.size());
    for (auto mutation : mutations) {
        values.push_back(static_cast<int>(mutation));
    }
    std::sort(values.begin(), values.end());

    std::ostringstream oss;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            oss << ',';
        }
        oss << values[i];
    }
    return oss.str();
}

struct SellGroup {
    std::string cropName;
    std::string mutationKey;
    std::vector<int> indices;
};

std::vector<SellGroup> groupHarvestBasket(const std::vector<BasketEntry>& basket) {
    std::vector<SellGroup> groups;

    for (int i = 0; i < static_cast<int>(basket.size()); ++i) {
        const BasketEntry& entry = basket[i];
        const std::string key = entry.cropName + "|" + makeMutationKey(entry.item.getMutationList());

        auto it = std::find_if(groups.begin(), groups.end(), [&](const SellGroup& group) {
            return group.cropName == entry.cropName && group.mutationKey == key;
        });

        if (it == groups.end()) {
            groups.push_back({entry.cropName, key, {i}});
        } else {
            it->indices.push_back(i);
        }
    }

    return groups;
}
} // namespace

// ─────────────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────────────
GameScreen::GameScreen(sf::RenderWindow& window, sf::Font& font)
    : window_(window), font_(font), game_(), shop_()
{
    catalogue_ = makeShopCatalogue();

    // 2 ticks/sec
    game_.getTickSystem().setTickRate(0.5f);

    // Starting resources: 100 sheckles + 5 carrot seeds
    game_.getPlayer().addSheckles(100.0f);
    auto seed = std::make_unique<Seed>(1, "Carrot Seed",
                                       "Grows into a carrot.", 10.0, "Carrot");
    game_.getPlayer().getInventory().addItem(std::move(seed), 5);

    setupShop();
}

void GameScreen::setupShop() {
    for (const auto& def : catalogue_) {
        if (def.type == ShopItemType::SEED)
            shop_.addAvailableItem(std::make_unique<Seed>(
                1, def.name, def.description, def.buyPrice, def.cropName));
    }
}

const ShopItemDef* GameScreen::findItem(const std::string& name) const {
    for (const auto& d : catalogue_)
        if (d.name == name) return &d;
    return nullptr;
}

const ShopItemDef* GameScreen::findItemByCrop(const std::string& cropName) const {
    for (const auto& d : catalogue_)
        if (d.cropName == cropName) return &d;
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────
//  Main loop
// ─────────────────────────────────────────────────────────────────────
void GameScreen::run() {
    sf::Clock clock;
    lastTick_ = game_.getTickSystem().getTick();

    while (window_.isOpen()) {
        while (true) {
            auto ev = window_.pollEvent();
            if (!ev.has_value()) break;
            handleEvent(*ev);
        }
        float dt = clock.restart().asSeconds();
        update(dt);
        render();
    }
}

// ─────────────────────────────────────────────────────────────────────
//  Events
// ─────────────────────────────────────────────────────────────────────
void GameScreen::handleEvent(const sf::Event& ev) {
    if (ev.is<sf::Event::Closed>()) { window_.close(); return; }

    if (auto* kp = ev.getIf<sf::Event::KeyPressed>()) {
        if (kp->code == sf::Keyboard::Key::Escape) {
            if (shopOpen_) shopOpen_ = false;
            else window_.close();
        }
        if (kp->code == sf::Keyboard::Key::F) {
            fastMode_ = !fastMode_;
            game_.getTickSystem().setTickRate(fastMode_ ? 0.05f : 0.5f);
            setStatus(fastMode_ ? "Speed: FAST" : "Speed: Normal", 1.5f);
        }
    }

    if (auto* mb = ev.getIf<sf::Event::MouseButtonReleased>()) {
        if (mb->button == sf::Mouse::Button::Left) {
            sf::Vector2f pos = window_.mapPixelToCoords(mb->position);
            if (shopOpen_) onShopClick(pos);
            else           onMouseClick(pos);
        }
    }
}

void GameScreen::onMouseClick(sf::Vector2f pos) {
    // Shop tab button
    float shopBtnX = BOARD_X + BOARD_W + 16.f;
    float shopBtnY = INV_BAR_Y;
    if (sf::FloatRect{{shopBtnX, shopBtnY},{INV_BAR_H, INV_BAR_H}}.contains(pos)) {
        shopOpen_ = true; return;
    }

    // Inventory bar slots
    {
        struct Slot { std::string name; bool isHarvest; };
        std::vector<Slot> slots;
        const auto& items = game_.getPlayer().getInventory().getItems();
        for (const auto& item : items) {
            slots.push_back({item->getName(), false});
        }
        slots.push_back({"__harvest__", true});

        int   n      = (int)slots.size();
        float barW   = BOARD_W;
        float slotSz = std::min(68.f, (barW - 8.f) / (float)n - 6.f);
        float totalW = n * (slotSz + 6.f) - 6.f;
        float startX = BOARD_X + (barW - totalW) / 2.f;
        float slotY  = INV_BAR_Y + (INV_BAR_H - slotSz) / 2.f;

        for (int i = 0; i < n; ++i) {
            float sx = startX + i * (slotSz + 6.f);
            if (sf::FloatRect{{sx, slotY},{slotSz, slotSz}}.contains(pos)) {
                if (slots[i].isHarvest) {
                    shopOpen_ = true;
                    shopTab_  = 2;   // jump straight to Sell tab
                } else {
                    const auto* def = findItem(slots[i].name);
                    if (def && def->type == ShopItemType::SEED) {
                        selectedSeed_ = slots[i].name;
                        equippedTool_ = "";
                        setStatus("Selected: " + def->cropName + " seed", 1.5f);
                    } else if (def && def->type == ShopItemType::TOOL) {
                        equippedTool_ = slots[i].name;
                        selectedSeed_ = "";
                        setStatus("Equipped: " + def->cropName, 1.5f);
                    }
                }
                return;
            }
        }
    }

    // Garden grid
    bool inGrid = (pos.x >= GRID_X && pos.x < GRID_X + BOARD_COLS*CELL_SZ &&
                   pos.y >= GRID_Y && pos.y < GRID_Y + BOARD_ROWS*CELL_SZ);
    if (inGrid) handleCellClick(pos);
}

// ─────────────────────────────────────────────────────────────────────
//  Update
// ─────────────────────────────────────────────────────────────────────
void GameScreen::update(float dt) {
    game_.update(dt);

    std::size_t newTick = game_.getTickSystem().getTick();
    if (newTick > lastTick_) {
        std::size_t delta = newTick - lastTick_;
        lastTick_ = newTick;
        dayCount_ = 1 + newTick / 60;

        Garden& g = game_.getGarden();
        for (int y = 0; y < g.getHeight(); ++y)
            for (int x = 0; x < g.getWidth(); ++x)
                if (Plant* p = g.getCell(x, y).getPlant())
                    p->grow(delta);

        game_.getWeatherSystem().update();
        audioManager_.updateBGM(game_.getWeatherSystem().getCurrentWeather());
        game_.getWeatherSystem().applyEffectsToGrid(g);
    }

    if (statusTimer_ > 0.f) statusTimer_ -= dt;
}

// ─────────────────────────────────────────────────────────────────────
//  Game actions
// ─────────────────────────────────────────────────────────────────────
void GameScreen::handleCellClick(sf::Vector2f pos) {
    int gx = (int)((pos.x - GRID_X) / CELL_SZ);
    int gy = (int)((pos.y - GRID_Y) / CELL_SZ);
    if (gx < 0 || gx >= (int)BOARD_COLS || gy < 0 || gy >= (int)BOARD_ROWS) return;

    Cell&  cell  = game_.getGarden().getCell(gx, gy);
    Plant* plant = cell.getPlant();

    if (!equippedTool_.empty() && plant) {
        useToolOnCell(gx, gy); return;
    }

    if (plant) {
        if (plant->isFullyGrown()) harvestCell(gx, gy);
        else setStatus("Growing... " + std::to_string(plant->getTimeToGrowth()) + " ticks left.");
    } else {
        if (selectedSeed_.empty()) {
            setStatus("Select a seed from your inventory first!");
        } else if (game_.getPlayer().getInventory().getQuantity(selectedSeed_) > 0) {
            plantSeed(gx, gy);
        } else {
            setStatus("No " + selectedSeed_ + "! Visit the shop.");
        }
    }
}

void GameScreen::plantSeed(int gx, int gy) {
    if (selectedSeed_.empty()) return;
    const auto* def = findItem(selectedSeed_);
    if (!def) {
        setStatus("Missing seed data for " + selectedSeed_);
        return;
    }

    game_.getPlayer().getInventory().removeItem(selectedSeed_, 1);
    game_.getGarden().plantCrop(gx, gy, makePlant(*def));
    setStatus("Planted " + (def ? def->cropName : selectedSeed_) + "!");

    if (game_.getPlayer().getInventory().getQuantity(selectedSeed_) == 0)
        selectedSeed_.clear();
}

void GameScreen::harvestCell(int gx, int gy) {
    Cell&  cell  = game_.getGarden().getCell(gx, gy);
    Plant* plant = cell.getPlant();
    if (!plant || !plant->isFullyGrown()) return;

    std::string plantName = plant->getName();
    HarvestedItem item    = plant->harvest();
    float         price   = (float)item.getPrice();
    harvestBasket_.push_back({std::move(item), plantName});

    if (plant->isFullyGrown()) cell.clearPlant();

    std::ostringstream ss;
    ss << "Harvested " << plantName << "!  +"
       << std::fixed << std::setprecision(0) << price << " S";
    const auto& back = harvestBasket_.back();
    if (!back.item.getMutationList().empty()) ss << " (" << back.item.getMutations() << ")";
    setStatus(ss.str(), 3.f);
}

void GameScreen::sellAll() {
    if (harvestBasket_.empty()) { setStatus("Harvest basket is empty!"); return; }
    float total = 0.f;
    for (const auto& e : harvestBasket_) total += (float)e.item.getPrice();
    harvestBasket_.clear();
    game_.getPlayer().addSheckles(total);
    std::ostringstream ss;
    ss << "Sold for +" << std::fixed << std::setprecision(0) << total << " sheckles!";
    setStatus(ss.str(), 3.f);
}

void GameScreen::sellOne(int index) {
    if (index < 0 || index >= (int)harvestBasket_.size()) return;
    float price = (float)harvestBasket_[index].item.getPrice();
    std::string crop = harvestBasket_[index].cropName;
    harvestBasket_.erase(harvestBasket_.begin() + index);
    game_.getPlayer().addSheckles(price);
    std::ostringstream ss;
    ss << "Sold " << crop << " for +" << std::fixed << std::setprecision(0) << price << " S!";
    setStatus(ss.str(), 2.5f);
}

void GameScreen::sellGroup(const std::vector<int>& indices) {
    if (indices.empty()) return;

    std::vector<int> sorted = indices;
    std::sort(sorted.begin(), sorted.end(), std::greater<int>());

    float total = 0.f;
    std::string crop = harvestBasket_[sorted.front()].cropName;
    int soldCount = 0;

    for (int index : sorted) {
        if (index < 0 || index >= static_cast<int>(harvestBasket_.size())) continue;
        total += static_cast<float>(harvestBasket_[index].item.getPrice());
        harvestBasket_.erase(harvestBasket_.begin() + index);
        ++soldCount;
    }

    if (soldCount == 0) return;

    game_.getPlayer().addSheckles(total);
    std::ostringstream ss;
    ss << "Sold x" << soldCount << " " << crop << " for +"
       << std::fixed << std::setprecision(0) << total << " S!";
    setStatus(ss.str(), 2.5f);
}

void GameScreen::useToolOnCell(int gx, int gy) {
    const auto* def = findItem(equippedTool_);
    if (!def || def->toolBoost == 0) return;

    Cell&  cell  = game_.getGarden().getCell(gx, gy);
    Plant* plant = cell.getPlant();
    if (!plant) return;

    plant->grow((std::size_t)def->toolBoost);
    game_.getPlayer().getInventory().removeItem(equippedTool_, 1);

    std::string toolName = def->cropName;
    std::string boost    = std::to_string(def->toolBoost);
    if (game_.getPlayer().getInventory().getQuantity(equippedTool_) == 0)
        equippedTool_.clear();

    setStatus("Used " + toolName + "! +" + boost + " growth ticks");
}

void GameScreen::buyItem(const ShopItemDef& def) {
    if (game_.getPlayer().getSheckles() < def.buyPrice) {
        setStatus("Not enough sheckles! (need " + std::to_string((int)def.buyPrice) + ")");
        return;
    }
    game_.getPlayer().deductSheckles(def.buyPrice);

    auto item = std::make_unique<Seed>(1, def.name, def.description,
                                       def.buyPrice, def.cropName);
    game_.getPlayer().getInventory().addItem(std::move(item), 1);

    if (def.type == ShopItemType::SEED) {
        selectedSeed_ = def.name;
        equippedTool_.clear();
    } else {
        equippedTool_ = def.name;
        selectedSeed_.clear();
    }

    setStatus("Bought " + def.cropName + "!", 2.5f);
}

void GameScreen::setStatus(const std::string& msg, float dur) {
    statusMsg_   = msg;
    statusTimer_ = dur;
}

// ─────────────────────────────────────────────────────────────────────
//  Master render
// ─────────────────────────────────────────────────────────────────────
void GameScreen::render() {
    sf::Vector2f mouse = window_.mapPixelToCoords(sf::Mouse::getPosition(window_));
    window_.clear(Pal::SKY);

    drawBackground();
    drawTopUI();
    drawGardenBoard(mouse);
    drawInventoryBar(mouse);
    drawShopTabButton(mouse);
    drawStatus();

    if (shopOpen_) drawShopOverlay(mouse);

    window_.display();
}

// ─────────────────────────────────────────────────────────────────────
//  Background
// ─────────────────────────────────────────────────────────────────────
void GameScreen::drawBackground() {
    sf::RectangleShape top({ 1920.f, 180.f });
    top.setFillColor({ 88, 162, 210 });
    window_.draw(top);

    sf::RectangleShape bot({ 1920.f, 200.f });
    bot.setPosition({ 0.f, 880.f });
    bot.setFillColor({ 98, 178, 215 });
    window_.draw(bot);

    const CloudDef clouds[] = {
        {130.f,  85.f, 1.00f},{370.f, 48.f, 0.72f},{1145.f,65.f,1.18f},
        {1545.f,105.f, 0.88f},{778.f, 38.f, 0.62f},{1350.f,40.f,0.55f},
    };
    for (auto& c : clouds) CropRenderer::drawCloud(window_, c.x, c.y, c.scale);
}

// ─────────────────────────────────────────────────────────────────────
//  Top UI
// ─────────────────────────────────────────────────────────────────────
void GameScreen::drawTopUI() {
    // Day
    {
        DrawUtils::drawPxPanel(window_, font_, {18.f, 14.f}, {210.f, 54.f});
        sf::CircleShape sun(13.f);
        sun.setFillColor({255, 210, 40});
        sun.setPosition({30.f, 23.f});
        window_.draw(sun);
        auto t = DrawUtils::makeText(font_, "DAY " + std::to_string(dayCount_), 24, Pal::DARKTEXT);
        t.setPosition({70.f, 27.f});
        window_.draw(t);
    }
    // Sheckles
    {
        sf::Vector2f sz{240.f, 54.f};
        sf::Vector2f pos{1920.f - sz.x - 18.f, 14.f};
        DrawUtils::drawPxPanel(window_, font_, pos, sz);
        sf::CircleShape coin(13.f);
        coin.setFillColor(Pal::GOLD);
        coin.setPosition({pos.x + 14.f, pos.y + 14.f});
        window_.draw(coin);
        auto dlr = DrawUtils::makeText(font_, "$", 15, Pal::DARKTEXT);
        dlr.setPosition({pos.x + 19.f, pos.y + 17.f});
        window_.draw(dlr);
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(0) << game_.getPlayer().getSheckles() << " S";
        auto t = DrawUtils::makeText(font_, ss.str(), 26, Pal::GOLD);
        auto lb = t.getLocalBounds();
        t.setPosition({pos.x + sz.x - lb.size.x - 14.f, pos.y + 14.f});
        window_.draw(t);
    }
    // Weather
    {
        static const char* WX[] = {"Summer","Rain","Frost","Thunder","Meteors"};
        WeatherType wt = game_.getWeatherSystem().getCurrentWeather();
        std::string ws = std::string(WX[(int)wt]);
        DrawUtils::drawPxPanel(window_, font_, {960.f - 160.f, 16.f}, {320.f, 46.f});
        auto t = DrawUtils::makeText(font_, ws, 20, Pal::DARKTEXT);
        auto lb = t.getLocalBounds();
        t.setPosition({960.f - lb.size.x/2.f, 28.f});
        window_.draw(t);
    }
    // Equipped tool indicator
    if (!equippedTool_.empty()) {
        const auto* def = findItem(equippedTool_);
        std::string label = "Tool: " + (def ? def->cropName : equippedTool_) + "  [ESC=close]";
        DrawUtils::drawPxPanel(window_, font_, {BOARD_X, 14.f}, {320.f, 46.f},
                    Pal::FRAME_MID, {40,80,160}, {80,140,220});
        auto t = DrawUtils::makeText(font_, label, 18, { 80,160,255 });
        t.setPosition({BOARD_X + 10.f, 27.f});
        window_.draw(t);
    }
}

// ─────────────────────────────────────────────────────────────────────
//  Garden board
// ─────────────────────────────────────────────────────────────────────
void GameScreen::drawGardenBoard(sf::Vector2f mouse) {
    // Drop shadow
    sf::RectangleShape s({BOARD_W+8.f, BOARD_H+8.f});
    s.setPosition({BOARD_X-2.f, BOARD_Y+4.f});
    s.setFillColor({20,10,2,150});
    window_.draw(s);

    sf::RectangleShape b({BOARD_W+6.f, BOARD_H+6.f});
    b.setPosition({BOARD_X-3.f, BOARD_Y-3.f});
    b.setFillColor(Pal::BOARD_DRK);
    window_.draw(b);

    sf::RectangleShape f({BOARD_W, BOARD_H});
    f.setPosition({BOARD_X, BOARD_Y});
    f.setFillColor(Pal::BOARD_MID);
    window_.draw(f);

    // Highlight
    sf::RectangleShape ht({BOARD_W, BOARD_PAD});
    ht.setPosition({BOARD_X, BOARD_Y});
    ht.setFillColor(Pal::BOARD_LIT);
    window_.draw(ht);
    sf::RectangleShape vl({BOARD_PAD, BOARD_H});
    vl.setPosition({BOARD_X, BOARD_Y});
    vl.setFillColor(Pal::BOARD_LIT);
    window_.draw(vl);

    // Shadow
    sf::RectangleShape hb({BOARD_W, BOARD_PAD});
    hb.setPosition({BOARD_X, BOARD_Y+BOARD_H-BOARD_PAD});
    hb.setFillColor(Pal::BOARD_DRK);
    window_.draw(hb);
    sf::RectangleShape vr({BOARD_PAD, BOARD_H});
    vr.setPosition({BOARD_X+BOARD_W-BOARD_PAD, BOARD_Y});
    vr.setFillColor(Pal::BOARD_DRK);
    window_.draw(vr);

    // Cells
    for (int gy = 0; gy < (int)BOARD_ROWS; ++gy)
        for (int gx = 0; gx < (int)BOARD_COLS; ++gx) {
            sf::Vector2f sc{GRID_X + gx*CELL_SZ, GRID_Y + gy*CELL_SZ};
            bool hov = (mouse.x>=sc.x && mouse.x<sc.x+CELL_SZ &&
                        mouse.y>=sc.y && mouse.y<sc.y+CELL_SZ);
            drawCell(gx, gy, sc, hov);
        }

}

void GameScreen::drawCell(int gx, int gy, sf::Vector2f s, bool hov) {
    const float GAP = 2.f, SZ = CELL_SZ - GAP;
    Cell&  cell  = game_.getGarden().getCell(gx, gy);
    Plant* plant = cell.getPlant();
    bool   ripe  = plant && plant->isFullyGrown();

    sf::Color base = plant ? Pal::SOIL_DRK : Pal::SOIL_MID;
    if (hov && !plant)  base = {160, 110, 56};
    if (hov && ripe)    base = {195, 178, 35};

    sf::RectangleShape sh({SZ, SZ});
    sh.setPosition({s.x+GAP, s.y+GAP});
    sh.setFillColor(Pal::SOIL_DRK);
    window_.draw(sh);

    sf::RectangleShape fc({SZ-2.f, SZ-2.f});
    fc.setPosition({s.x+GAP, s.y+GAP});
    fc.setFillColor(base);
    window_.draw(fc);

    sf::RectangleShape hl({SZ-2.f, 3.f});
    hl.setPosition({s.x+GAP, s.y+GAP});
    hl.setFillColor(Pal::SOIL_LIT);
    window_.draw(hl);
    sf::RectangleShape vl({3.f, SZ-2.f});
    vl.setPosition({s.x+GAP, s.y+GAP});
    vl.setFillColor(Pal::SOIL_LIT);
    window_.draw(vl);

    if (plant) CropRenderer::drawPlant(window_, font_, plant, s, CELL_SZ, catalogue_);

    if (ripe) {
        sf::RectangleShape gl({SZ-2.f, SZ-2.f});
        gl.setPosition({s.x+GAP, s.y+GAP});
        gl.setFillColor(sf::Color::Transparent);
        gl.setOutlineThickness(4.f);
        gl.setOutlineColor({255,220,30,180});
        window_.draw(gl);
    }

    if (plant && !plant->getMutations().empty()) {
        sf::CircleShape dot(6.f);
        dot.setFillColor(Pal::MUTATION);
        dot.setPosition({s.x+CELL_SZ-16.f, s.y+4.f});
        window_.draw(dot);
    }
}

// ─────────────────────────────────────────────────────────────────────
//  Inventory bar
// ─────────────────────────────────────────────────────────────────────
void GameScreen::drawInventoryBar(sf::Vector2f mouse) {
    struct Slot { std::string name; int qty; bool isHarvest; };
    std::vector<Slot> slots;
    const auto& items = game_.getPlayer().getInventory().getItems();
    for (const auto& item : items) {
        int q = game_.getPlayer().getInventory().getQuantity(item->getName());
        slots.push_back({item->getName(), q, false});
    }
    slots.push_back({"__harvest__", (int)harvestBasket_.size(), true});
    int   n      = (int)slots.size();
    float barW   = BOARD_W;
    float slotSz = std::min(72.f, (barW - 8.f)/(float)n - 6.f);
    float totalW = n * (slotSz + 6.f) - 6.f;
    float startX = BOARD_X + (barW - totalW) / 2.f;
    float slotY  = INV_BAR_Y + (INV_BAR_H - slotSz) / 2.f;

    DrawUtils::drawPxPanel(window_, font_, {BOARD_X, INV_BAR_Y}, {BOARD_W, INV_BAR_H});

    for (int i = 0; i < n; ++i) {
        float sx  = startX + i * (slotSz + 6.f);
        bool  hov = (mouse.x>=sx && mouse.x<sx+slotSz &&
                     mouse.y>=slotY && mouse.y<slotY+slotSz);
        const Slot& sl  = slots[i];
        bool        sel = (!sl.isHarvest &&
                           (sl.name == selectedSeed_ || sl.name == equippedTool_));

        // Slot bg
        sf::RectangleShape shadow({slotSz, slotSz});
        shadow.setPosition({sx, slotY});
        shadow.setFillColor(Pal::SOIL_DRK);
        window_.draw(shadow);

        sf::Color face = hov ? sf::Color{160,112,58} : Pal::SOIL_MID;
        sf::RectangleShape fc({slotSz-3.f, slotSz-3.f});
        fc.setPosition({sx, slotY});
        fc.setFillColor(face);
        window_.draw(fc);

        if (sl.isHarvest) {
            // Harvest basket icon
            float ico = slotSz * 0.42f;
            sf::ConvexShape hv(3);
            hv.setPoint(0,{sx+slotSz/2.f-ico*0.5f, slotY+slotSz/2.f-ico*0.3f});
            hv.setPoint(1,{sx+slotSz/2.f+ico*0.5f, slotY+slotSz/2.f-ico*0.3f});
            hv.setPoint(2,{sx+slotSz/2.f,           slotY+slotSz/2.f+ico*0.7f});
            hv.setFillColor({220,110,25});
            window_.draw(hv);
            if (sl.qty > 0) {
                auto cnt = DrawUtils::makeText(font_, std::to_string(sl.qty), 16, Pal::GOLD);
                cnt.setPosition({sx+slotSz-20.f, slotY+slotSz-22.f});
                window_.draw(cnt);
                if (hov) {
                    auto tip = DrawUtils::makeText(font_, "Click: SELL ALL", 17, Pal::GOLD);
                    tip.setPosition({sx-20.f, slotY-26.f});
                    window_.draw(tip);
                }
            }
            auto lbl = DrawUtils::makeText(font_, "SELL", 12, Pal::CREAM);
            lbl.setPosition({sx+slotSz/2.f-14.f, slotY+slotSz-18.f});
            window_.draw(lbl);
        } else {
            const auto* def = findItem(sl.name);
            if (def) CropRenderer::drawCropIcon(window_, font_, *def, {sx+slotSz/2.f, slotY+slotSz*0.46f}, slotSz*0.72f);
            auto cnt = DrawUtils::makeText(font_, std::to_string(sl.qty), 15, Pal::GOLD);
            cnt.setPosition({sx+slotSz-20.f, slotY+slotSz-22.f});
            window_.draw(cnt);
        }

        if (sel || hov) {
            sf::RectangleShape outline({slotSz-3.f, slotSz-3.f});
            outline.setPosition({sx, slotY});
            outline.setFillColor(sf::Color::Transparent);
            outline.setOutlineThickness(3.f);
            outline.setOutlineColor(sel ? Pal::GOLD : sf::Color{200,200,200,140});
            window_.draw(outline);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────
//  Shop tab button
// ─────────────────────────────────────────────────────────────────────
void GameScreen::drawShopTabButton(sf::Vector2f mouse) {
    float bx  = BOARD_X + BOARD_W + 16.f;
    float by  = INV_BAR_Y;
    float bsz = INV_BAR_H;
    bool  hov = (mouse.x>=bx && mouse.x<bx+bsz && mouse.y>=by && mouse.y<by+bsz);

    DrawUtils::drawPxPanel(window_, font_, {bx,by},{bsz,bsz});
    for (int i=0;i<3;++i){
        sf::RectangleShape shelf({bsz-20.f,7.f});
        shelf.setPosition({bx+10.f, by+12.f+i*18.f});
        shelf.setFillColor(hov ? Pal::GOLD : Pal::CREAM);
        window_.draw(shelf);
    }
    auto lbl = DrawUtils::makeText(font_, "SHOP",14, hov ? Pal::GOLD : Pal::DARKTEXT);
    lbl.setPosition({bx+bsz/2.f-18.f, by+bsz-20.f});
    window_.draw(lbl);
}

// ─────────────────────────────────────────────────────────────────────
//  Shop overlay
// ─────────────────────────────────────────────────────────────────────
void GameScreen::drawShopOverlay(sf::Vector2f mouse) {
    // Dim
    sf::RectangleShape dim({1920.f,1080.f});
    dim.setFillColor({0,0,0,190});
    window_.draw(dim);

    // Main panel
    DrawUtils::drawPxPanel(window_, font_, {SHOP_X, SHOP_Y},{SHOP_W, SHOP_H},
                Pal::SHOP_BG, {25,14,4}, {75,48,18});

    // Title bar
    {
        sf::RectangleShape tbar({SHOP_W, 62.f});
        tbar.setPosition({SHOP_X, SHOP_Y});
        tbar.setFillColor({38,22,8});
        window_.draw(tbar);

        // Decorative corner coins
        for (float cx : {SHOP_X+24.f, SHOP_X+SHOP_W-38.f}) {
            sf::CircleShape coin(14.f);
            coin.setFillColor(Pal::GOLD);
            coin.setPosition({cx, SHOP_Y+17.f});
            window_.draw(coin);
            auto ds = DrawUtils::makeText(font_, "$", 15, Pal::DARKTEXT);
            ds.setPosition({cx+5.f, SHOP_Y+20.f});
            window_.draw(ds);
        }

        auto t = DrawUtils::makeText(font_, "SHOP", 36, Pal::GOLD);
        auto lb = t.getLocalBounds();
        t.setPosition({SHOP_X + SHOP_W/2.f - lb.size.x/2.f, SHOP_Y + 10.f});
        window_.draw(t);

        // Close
        float cx = SHOP_X+SHOP_W-56.f, cy = SHOP_Y+8.f;
        bool  chov = (mouse.x>=cx&&mouse.x<cx+42.f&&mouse.y>=cy&&mouse.y<cy+42.f);
        sf::RectangleShape cb({42.f,42.f});
        cb.setPosition({cx,cy});
        cb.setFillColor(chov ? sf::Color{210,55,55} : sf::Color{150,38,38});
        window_.draw(cb);
        // X highlight
        sf::RectangleShape chl({42.f,3.f});
        chl.setPosition({cx,cy});
        chl.setFillColor({255,255,255,50});
        window_.draw(chl);
        auto xb = DrawUtils::makeText(font_, "X", 26, Pal::CREAM);
        xb.setPosition({cx+11.f, cy+7.f});
        window_.draw(xb);
    }

    // Tab buttons — Seeds / Tools / Sell
    float tabY = SHOP_Y + 66.f, tabH = 44.f, tabW = 180.f;
    const char* tabs[] = {"SEEDS","TOOLS","SELL"};
    for (int t = 0; t < 3; ++t) {
        float tx   = SHOP_X + 30.f + t*(tabW+10.f);
        bool  act  = (shopTab_==t);
        bool  thov = (mouse.x>=tx&&mouse.x<tx+tabW&&mouse.y>=tabY&&mouse.y<tabY+tabH);
        sf::Color tc = act ? Pal::TAB_ACT : (thov ? sf::Color{130,82,35} : Pal::TAB_INACT);
        // SELL tab tinted green when basket has items
        if (t == 2 && !harvestBasket_.empty())
            tc = act ? sf::Color{60,170,60} : (thov ? sf::Color{50,140,50} : sf::Color{35,100,35});
        sf::RectangleShape tb({tabW,tabH});
        tb.setPosition({tx,tabY});
        tb.setFillColor(tc);
        window_.draw(tb);
        if (act) {
            sf::RectangleShape ul({tabW,4.f});
            ul.setPosition({tx,tabY+tabH-4.f});
            ul.setFillColor(Pal::GOLD);
            window_.draw(ul);
        }
        // Badge showing item count on SELL tab
        auto tl = DrawUtils::makeText(font_, tabs[t], 22, act ? Pal::GOLD : Pal::CREAM);
        auto lb = tl.getLocalBounds();
        tl.setPosition({tx+(tabW-lb.size.x)/2.f, tabY+10.f});
        window_.draw(tl);
        if (t == 2 && !harvestBasket_.empty()) {
            auto badge = DrawUtils::makeText(font_, std::to_string(harvestBasket_.size()), 14, Pal::GOLD);
            badge.setPosition({tx+tabW-22.f, tabY+4.f});
            window_.draw(badge);
        }
    }

    // Divider
    sf::RectangleShape div({SHOP_W-60.f,2.f});
    div.setPosition({SHOP_X+30.f, SHOP_Y+114.f});
    div.setFillColor({75,48,18});
    window_.draw(div);

    float cardsY = SHOP_Y + 122.f;
    float cardsH = SHOP_H - 122.f - 14.f;
    float cardsX = SHOP_X + 22.f;
    float cardsW = SHOP_W - 44.f;

    if (shopTab_ == 0) {        // Seed cards: 4 columns
        std::vector<const ShopItemDef*> seeds;
        for (const auto& d : catalogue_)
            if (d.type == ShopItemType::SEED) seeds.push_back(&d);

        int   cols = 4;
        int   n    = (int)seeds.size();
        int   rows = (n + cols-1) / cols;
        float cw   = (cardsW - (cols-1)*10.f) / cols;
        float ch   = std::min(175.f, (cardsH - (rows-1)*10.f) / rows);

        for (int i = 0; i < n; ++i) {
            sf::FloatRect b = getSeedCardRect(i, cols, cardsX, cardsY, cw, ch);
            drawSeedCard(*seeds[i], b, mouse);
        }
    } else if (shopTab_ == 1) {
        // Tool cards: 2 large
        std::vector<const ShopItemDef*> tools;
        for (const auto& d : catalogue_)
            if (d.type == ShopItemType::TOOL) tools.push_back(&d);

        float cw = (cardsW - 20.f) / 2.f;
        float ch = std::min(260.f, cardsH);
        for (int i = 0; i < (int)tools.size(); ++i) {
            sf::FloatRect b = getToolCardRect(i, cardsX, cardsY, cw, ch);
            drawToolCard(*tools[i], b, mouse);
        }
    } else {
        drawSellPage(mouse);
    }
}

// ─── Seed card ────────────────────────────────────────────────────────
void GameScreen::drawSeedCard(const ShopItemDef& def, sf::FloatRect b, sf::Vector2f mouse) {
    bool hov   = b.contains(mouse);
    bool owned = (game_.getPlayer().getInventory().getQuantity(def.name) > 0);

    sf::RectangleShape bg(b.size);
    bg.setPosition(b.position);
    bg.setFillColor(hov ? Pal::CARD_HOV : Pal::CARD_BG);
    window_.draw(bg);

    // Border
    sf::RectangleShape border(b.size);
    border.setPosition(b.position);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineThickness(2.f);
    border.setOutlineColor(hov ? Pal::GOLD : sf::Color{75,48,18});
    window_.draw(border);

    float iconSz = b.size.y * 0.52f;
    float iconX  = b.position.x + b.size.x * 0.5f;
    float iconY  = b.position.y + iconSz * 0.56f;

    // Icon dark bg
    sf::RectangleShape iconBg({iconSz, iconSz});
    iconBg.setPosition({iconX - iconSz/2.f, b.position.y});
    iconBg.setFillColor({30,18,6});
    window_.draw(iconBg);

    CropRenderer::drawCropIcon(window_, font_, def, {iconX, iconY}, iconSz * 0.78f);

    float ty    = b.position.y + iconSz + 6.f;
    float textW = b.size.x - 8.f;

    auto name = DrawUtils::makeText(font_, def.cropName, 19, owned ? Pal::GOLD : Pal::CREAM);
    auto nlb  = name.getLocalBounds();
    name.setPosition({b.position.x + (textW - nlb.size.x)/2.f + 4.f, ty});
    window_.draw(name);

    auto desc = DrawUtils::makeText(font_, def.description, 13, {180,150,110});
    auto dlb  = desc.getLocalBounds();
    desc.setPosition({b.position.x + (textW - dlb.size.x)/2.f + 4.f, ty + 23.f});
    window_.draw(desc);

    std::ostringstream stats;
    stats << "Sells " << (int)def.sellPrice << " S";
    auto st = DrawUtils::makeText(font_, stats.str(), 13, {150,210,130});
    auto slb = st.getLocalBounds();
    st.setPosition({b.position.x + (textW - slb.size.x)/2.f + 4.f, ty + 42.f});
    window_.draw(st);

    // Footer
    float footerY = b.position.y + b.size.y - 34.f;
    sf::RectangleShape footer({b.size.x, 34.f});
    footer.setPosition({b.position.x, footerY});
    footer.setFillColor({30,18,6});
    window_.draw(footer);

    auto price = DrawUtils::makeText(font_, std::to_string((int)def.buyPrice) + " S", 18, Pal::GOLD);
    price.setPosition({b.position.x + 8.f, footerY + 7.f});
    window_.draw(price);

    float btnW = 68.f, btnH = 26.f;
    sf::FloatRect btnR{{b.position.x+b.size.x-btnW-6.f, footerY+4.f},{btnW,btnH}};
    bool canAfford = (game_.getPlayer().getSheckles() >= def.buyPrice);
    DrawUtils::drawPxButton(window_, font_, btnR, "BUY",
                 canAfford ? Pal::BTN_BUY : sf::Color{80,80,80},
                 canAfford ? Pal::BTN_BHOV : sf::Color{80,80,80},
                 mouse, 18);

    if (owned) {
        int q = game_.getPlayer().getInventory().getQuantity(def.name);
        auto ob = DrawUtils::makeText(font_, "x"+std::to_string(q), 14, Pal::GOLD);
        ob.setPosition({b.position.x+4.f, b.position.y+4.f});
        window_.draw(ob);
    }
}

// ─── Tool card ────────────────────────────────────────────────────────
void GameScreen::drawToolCard(const ShopItemDef& def, sf::FloatRect b, sf::Vector2f mouse) {
    bool hov   = b.contains(mouse);
    bool owned = (game_.getPlayer().getInventory().getQuantity(def.name) > 0);

    sf::RectangleShape bg(b.size);
    bg.setPosition(b.position);
    bg.setFillColor(hov ? Pal::CARD_HOV : Pal::CARD_BG);
    window_.draw(bg);

    sf::RectangleShape border(b.size);
    border.setPosition(b.position);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineThickness(2.f);
    border.setOutlineColor(hov ? Pal::GOLD : sf::Color{75,48,18});
    window_.draw(border);

    float iconSz = b.size.y * 0.62f;
    float iconX  = b.position.x + iconSz*0.62f;
    float iconY  = b.position.y + b.size.y*0.42f;

    sf::RectangleShape iconBg({iconSz, iconSz});
    iconBg.setPosition({b.position.x, b.position.y + (b.size.y-iconSz)/2.f});
    iconBg.setFillColor({30,18,6});
    window_.draw(iconBg);
    CropRenderer::drawCropIcon(window_, font_, def, {iconX, iconY}, iconSz*0.72f);

    float tx = b.position.x + iconSz + 16.f;
    float ty = b.position.y + 20.f;

    auto name = DrawUtils::makeText(font_, def.cropName, 26, Pal::GOLD);
    name.setPosition({tx, ty});
    window_.draw(name);

    auto desc = DrawUtils::makeText(font_, def.description, 18, Pal::CREAM);
    desc.setPosition({tx, ty+38.f});
    window_.draw(desc);

    auto bst = DrawUtils::makeText(font_, "Boost: +" + std::to_string(def.toolBoost) + " growth ticks per use", 16, {150,210,130});
    bst.setPosition({tx, ty+70.f});
    window_.draw(bst);

    float footerY = b.position.y + b.size.y - 48.f;
    auto  pr      = DrawUtils::makeText(font_, std::to_string((int)def.buyPrice)+" S", 22, Pal::GOLD);
    pr.setPosition({tx, footerY});
    window_.draw(pr);

    float btnW=110.f, btnH=34.f;
    sf::FloatRect btnR{{b.position.x+b.size.x-btnW-10.f, footerY-4.f},{btnW,btnH}};
    bool canAfford = (game_.getPlayer().getSheckles()>=def.buyPrice);
    DrawUtils::drawPxButton(window_, font_, btnR,"BUY",
                 canAfford?Pal::BTN_BUY:sf::Color{80,80,80},
                 canAfford?Pal::BTN_BHOV:sf::Color{80,80,80},
                 mouse, 20);

    if (owned) {
        int q = game_.getPlayer().getInventory().getQuantity(def.name);
        auto ob = DrawUtils::makeText(font_, "x"+std::to_string(q)+" owned", 16, Pal::GOLD);
        ob.setPosition({b.position.x+4.f, b.position.y+4.f});
        window_.draw(ob);
    }
}

// ─── Sell page ────────────────────────────────────────────────────────
void GameScreen::drawSellPage(sf::Vector2f mouse) {
    float cardsY = SHOP_Y + 122.f;
    float cardsH = SHOP_H - 122.f - 14.f;
    float cardsX = SHOP_X + 22.f;
    float cardsW = SHOP_W - 44.f;
    const auto groups = groupHarvestBasket(harvestBasket_);

    // ── Empty state ───────────────────────────────────────────────────
    if (groups.empty()) {
        // Faded basket icon
        float cx = SHOP_X + SHOP_W / 2.f;
        float cy = SHOP_Y + SHOP_H / 2.f - 40.f;
        float ico = 80.f;
        sf::ConvexShape tri(3);
        tri.setPoint(0,{cx-ico*0.6f, cy-ico*0.2f});
        tri.setPoint(1,{cx+ico*0.6f, cy-ico*0.2f});
        tri.setPoint(2,{cx,           cy+ico*0.7f});
        tri.setFillColor({80,50,20,120});
        window_.draw(tri);
        auto msg = DrawUtils::makeText(font_, "Your harvest basket is empty!", 28, {140,100,55});
        auto lb  = msg.getLocalBounds();
        msg.setPosition({cx - lb.size.x/2.f, cy + ico*0.8f});
        window_.draw(msg);
        auto sub = DrawUtils::makeText(font_, "Harvest fully-grown crops from your garden first.", 20, {100,70,35});
        auto lb2 = sub.getLocalBounds();
        sub.setPosition({cx - lb2.size.x/2.f, cy + ico*0.8f + 40.f});
        window_.draw(sub);
        return;
    }

    // ── Header strip: total value + SELL ALL button ───────────────────
    float headerH = 52.f;
    sf::RectangleShape hdr({cardsW, headerH});
    hdr.setPosition({cardsX, cardsY});
    hdr.setFillColor({30,18,6});
    window_.draw(hdr);
    // Highlight
    sf::RectangleShape hdrHl({cardsW, 3.f});
    hdrHl.setPosition({cardsX, cardsY});
    hdrHl.setFillColor({80,55,20});
    window_.draw(hdrHl);

    // Compute total
    float total = 0.f;
    for (const auto& e : harvestBasket_) total += (float)e.item.getPrice();

    auto totalTxt = DrawUtils::makeText(font_, "Basket: " + std::to_string((int)harvestBasket_.size()) +
                             " items  |  Total: " +
                             [&]{ std::ostringstream s;
                                  s << std::fixed << std::setprecision(0) << total;
                                  return s.str(); }() + " S",
                             22, Pal::CREAM);
    totalTxt.setPosition({cardsX + 16.f, cardsY + 14.f});
    window_.draw(totalTxt);

    // SELL ALL button (right side of header)
    float saW = 160.f, saH = 36.f;
    sf::FloatRect saBtn{{cardsX + cardsW - saW - 10.f, cardsY + 8.f},{saW, saH}};
    DrawUtils::drawPxButton(window_, font_, saBtn, "SELL ALL",
                 {170, 110, 20}, {210, 140, 25}, mouse, 20);

    // ── Item grid ─────────────────────────────────────────────────────
    float gridY   = cardsY + headerH + 10.f;
    float gridH   = cardsH - headerH - 10.f;
    int   cols    = 5;
    int   n       = static_cast<int>(groups.size());
    int   rows    = (n + cols - 1) / cols;
    float cardW   = (cardsW - (cols - 1) * 10.f) / (float)cols;
    float cardH   = std::min(150.f, (gridH - (rows - 1) * 10.f) / (float)rows);

    for (int i = 0; i < n; ++i) {
        sf::FloatRect b = getSellCardRect(i, cols, cardsX, gridY, cardW, cardH);
        bool  hov = b.contains(mouse);

        const SellGroup& group = groups[i];
        const BasketEntry& entry = harvestBasket_[group.indices.front()];
        float price = (float)entry.item.getPrice();
        bool  hasMut = !entry.item.getMutationList().empty();
        int   count = static_cast<int>(group.indices.size());

        // Card background
        sf::RectangleShape bg(b.size);
        bg.setPosition(b.position);
        bg.setFillColor(hov ? Pal::CARD_HOV : Pal::CARD_BG);
        window_.draw(bg);

        // Border — gold if mutated
        sf::RectangleShape border(b.size);
        border.setPosition(b.position);
        border.setFillColor(sf::Color::Transparent);
        border.setOutlineThickness(2.f);
        border.setOutlineColor(hasMut ? Pal::MUTATION
                               : (hov ? Pal::GOLD : sf::Color{75,48,18}));
        window_.draw(border);

        // Crop icon (left third)
        float iconSz = cardH * 0.72f;
        float iconCX = b.position.x + iconSz * 0.56f;
        float iconCY = b.position.y + cardH * 0.46f;
        sf::RectangleShape iconBg({iconSz, iconSz});
        iconBg.setPosition({b.position.x, b.position.y + (cardH - iconSz)/2.f});
        iconBg.setFillColor({28,16,5});
        window_.draw(iconBg);

        const auto* def = findItemByCrop(entry.cropName);
        if (def) CropRenderer::drawCropIcon(window_, font_, *def, {iconCX, iconCY}, iconSz * 0.70f);

        // Text area (right of icon)
        float tx = b.position.x + iconSz + 10.f;
        float tw = cardW - iconSz - 14.f;
        float ty = b.position.y + 12.f;

        // Crop name
        auto nameT = DrawUtils::makeText(font_, entry.cropName, 20, hov ? Pal::GOLD : Pal::CREAM);
        nameT.setPosition({tx, ty});
        window_.draw(nameT);

        if (count > 1) {
            auto countT = DrawUtils::makeText(font_, "x" + std::to_string(count), 15, Pal::GOLD);
            countT.setPosition({tx + tw - 24.f, ty - 2.f});
            window_.draw(countT);
        }

        // Price
        std::ostringstream ps;
        ps << std::fixed << std::setprecision(0) << price << " S";
        auto priceT = DrawUtils::makeText(font_, ps.str(), 18, {150, 210, 130});
        priceT.setPosition({tx, ty + 26.f});
        window_.draw(priceT);

        // Mutation badge
        if (hasMut) {
            auto mutT = DrawUtils::makeText(font_, entry.item.getMutations(), 13, Pal::MUTATION);
            mutT.setPosition({tx, ty + 50.f});
            window_.draw(mutT);
        }

        // SELL button — bottom-right of card
        sf::FloatRect btnR = getSellButtonRect(b, iconSz);
        DrawUtils::drawPxButton(window_, font_, btnR, count > 1 ? ("SELL x" + std::to_string(count)) : "SELL",
                     {170, 110, 20}, {215, 145, 28},
                     mouse, 16);
    }
}

// ─── Shop click ───────────────────────────────────────────────────────
void GameScreen::onShopClick(sf::Vector2f pos) {
    // Close button
    float cx=SHOP_X+SHOP_W-56.f, cy=SHOP_Y+8.f;
    if (pos.x>=cx&&pos.x<cx+42.f&&pos.y>=cy&&pos.y<cy+42.f) {
        shopOpen_=false; return;
    }
    // Tab buttons (3 tabs)
    float tabY=SHOP_Y+66.f, tabH=44.f, tabW=180.f;
    for (int t=0;t<3;++t) {
        float tx=SHOP_X+30.f+t*(tabW+10.f);
        if (pos.x>=tx&&pos.x<tx+tabW&&pos.y>=tabY&&pos.y<tabY+tabH) {
            shopTab_=t; return;
        }
    }

    float cardsY=SHOP_Y+122.f, cardsH=SHOP_H-122.f-14.f;
    float cardsX=SHOP_X+22.f,  cardsW=SHOP_W-44.f;

    if (shopTab_==0) {
        // Seed BUY buttons
        std::vector<const ShopItemDef*> seeds;
        for (const auto& d:catalogue_) if(d.type==ShopItemType::SEED) seeds.push_back(&d);
        int cols=4, n=(int)seeds.size(), rows=(n+cols-1)/cols;
        float cw=(cardsW-(cols-1)*10.f)/cols;
        float ch=std::min(175.f,(cardsH-(rows-1)*10.f)/rows);
        for (int i=0;i<n;++i) {
            sf::FloatRect cardR = getSeedCardRect(i, cols, cardsX, cardsY, cw, ch);
            sf::FloatRect btnR = getSeedButtonRect(cardR);
            if (btnR.contains(pos)) { buyItem(*seeds[i]); return; }
        }
    } else if (shopTab_==1) {
        // Tool BUY buttons
        std::vector<const ShopItemDef*> tools;
        for (const auto& d:catalogue_) if(d.type==ShopItemType::TOOL) tools.push_back(&d);
        float cw=(cardsW-20.f)/2.f, ch=std::min(260.f,cardsH);
        for (int i=0;i<(int)tools.size();++i) {
            sf::FloatRect cardR = getToolCardRect(i, cardsX, cardsY, cw, ch);
            sf::FloatRect btnR = getToolButtonRect(cardR);
            if (btnR.contains(pos)) { buyItem(*tools[i]); return; }
        }
    } else {
        // Sell page
        const auto groups = groupHarvestBasket(harvestBasket_);
        if (groups.empty()) return;

        float headerH = 52.f;

        // SELL ALL button
        float saW=160.f, saH=36.f;
        sf::FloatRect saBtn{{cardsX+cardsW-saW-10.f, cardsY+8.f},{saW,saH}};
        if (saBtn.contains(pos)) { sellAll(); return; }

        // Individual SELL buttons
        float gridY = cardsY + headerH + 10.f;
        float gridH = cardsH - headerH - 10.f;
        int   cols  = 5;
        int   n     = static_cast<int>(groups.size());
        int   rows  = (n + cols - 1) / cols;
        float cardW = (cardsW - (cols-1)*10.f) / (float)cols;
        float cardH = std::min(150.f, (gridH - (rows-1)*10.f) / (float)rows);

        for (int i = 0; i < n; ++i) {
            sf::FloatRect cardR = getSellCardRect(i, cols, cardsX, gridY, cardW, cardH);
            float iconSz = cardH * 0.72f;
            sf::FloatRect btnR = getSellButtonRect(cardR, iconSz);
            if (btnR.contains(pos)) { sellGroup(groups[i].indices); return; }
        }
    }
}

sf::FloatRect GameScreen::getSeedCardRect(int i, int cols, float cardsX, float cardsY, float cardW, float cardH) const {
    int col = i % cols;
    int row = i / cols;
    return sf::FloatRect{{cardsX + col * (cardW + 10.f), cardsY + row * (cardH + 10.f)}, {cardW, cardH}};
}

sf::FloatRect GameScreen::getSeedButtonRect(sf::FloatRect cardRect) const {
    constexpr float btnW = 68.f;
    constexpr float btnH = 26.f;
    float footerY = cardRect.position.y + cardRect.size.y - 34.f;
    return sf::FloatRect{{cardRect.position.x + cardRect.size.x - btnW - 6.f, footerY + 4.f}, {btnW, btnH}};
}

sf::FloatRect GameScreen::getToolCardRect(int i, float cardsX, float cardsY, float cardW, float cardH) const {
    return sf::FloatRect{{cardsX + i * (cardW + 20.f), cardsY + 20.f}, {cardW, cardH}};
}

sf::FloatRect GameScreen::getToolButtonRect(sf::FloatRect cardRect) const {
    constexpr float btnW = 110.f;
    constexpr float btnH = 34.f;
    float footerY = cardRect.position.y + cardRect.size.y - 48.f;
    return sf::FloatRect{{cardRect.position.x + cardRect.size.x - btnW - 10.f, footerY - 4.f}, {btnW, btnH}};
}

sf::FloatRect GameScreen::getSellCardRect(int i, int cols, float cardsX, float gridY, float cardW, float cardH) const {
    int col = i % cols;
    int row = i / cols;
    return sf::FloatRect{{cardsX + col * (cardW + 10.f), gridY + row * (cardH + 10.f)}, {cardW, cardH}};
}

sf::FloatRect GameScreen::getSellButtonRect(sf::FloatRect cardRect, float iconSz) const {
    float tx = cardRect.position.x + iconSz + 10.f;
    float btnW = cardRect.size.x - iconSz - 14.f;
    constexpr float btnH = 26.f;
    return sf::FloatRect{{tx, cardRect.position.y + cardRect.size.y - btnH - 6.f}, {btnW, btnH}};
}

// ─────────────────────────────────────────────────────────────────────
//  Status bar
// ─────────────────────────────────────────────────────────────────────
void GameScreen::drawStatus() {
    if (statusTimer_<=0.f) return;
    float alpha=std::min(1.f, statusTimer_/0.4f);
    sf::Color bg{30,14,4,(uint8_t)(alpha*210)};
    sf::Color fg{255,228,100,(uint8_t)(alpha*255)};
    auto t=DrawUtils::makeText(font_, statusMsg_,22,fg);
    auto lb=t.getLocalBounds();
    float tw=lb.size.x+32.f;
    float tx=960.f-tw/2.f, ty=INV_BAR_Y-38.f;
    sf::RectangleShape pill({tw,30.f});
    pill.setPosition({tx,ty});
    pill.setFillColor(bg);
    window_.draw(pill);
    t.setPosition({tx+16.f,ty+4.f});
    window_.draw(t);
}
