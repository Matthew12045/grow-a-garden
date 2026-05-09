#include "GameScreen.h"
#include "CropRenderer.h"

#include "../items/FertilizerTool.h"
#include "../items/WateringCan.h"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────────────
GameScreen::GameScreen(sf::RenderWindow& window, sf::Font& font)
    : window_(window), font_(font), game_(), shop_(), catalogue_(makeShopCatalogue()),
      session_(game_, harvestBasket_, catalogue_),
      shopOverlay_(window_, font_, game_, harvestBasket_, catalogue_, *this),
      inventoryOverlay_(window_, font_, game_, catalogue_, *this)
{
    // 2 ticks/sec
    game_.getTickSystem().setTickRate(0.5f);

    session_.initOrLoad();

    if (game_.getPlayer().getInventory().getQuantity(selectedSeed_) == 0) {
        selectedSeed_.clear();
    }

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

    session_.save();
}

// ─────────────────────────────────────────────────────────────────────
//  Events
// ─────────────────────────────────────────────────────────────────────
void GameScreen::handleEvent(const sf::Event& ev) {
    if (ev.is<sf::Event::Closed>()) { window_.close(); return; }

    if (auto* kp = ev.getIf<sf::Event::KeyPressed>()) {
        if (kp->code == sf::Keyboard::Key::Escape) {
            if (inventoryOverlay_.isOpen()) inventoryOverlay_.close();
            else if (shopOverlay_.isOpen()) shopOverlay_.close();
            else window_.close();
        }
        if (kp->code == sf::Keyboard::Key::F) {
            fastMode_ = !fastMode_;
            game_.getTickSystem().setTickRate(fastMode_ ? 0.05f : 0.5f);
            setStatus(fastMode_ ? "Speed: FAST" : "Speed: Normal", 1.5f);
        }
    }

    if (auto* mb = ev.getIf<sf::Event::MouseButtonPressed>()) {
        if (mb->button == sf::Mouse::Button::Left && inventoryOverlay_.isOpen()) {
            sf::Vector2f pos = window_.mapPixelToCoords(mb->position);
            inventoryOverlay_.beginDrag(pos);
            return;
        }
    }

    if (auto* mb = ev.getIf<sf::Event::MouseButtonReleased>()) {
        if (mb->button == sf::Mouse::Button::Left) {
            sf::Vector2f pos = window_.mapPixelToCoords(mb->position);
            if (inventoryOverlay_.isOpen()) inventoryOverlay_.finishDrag(pos);
            else if (shopOverlay_.isOpen()) shopOverlay_.handleClick(pos);
            else                  onMouseClick(pos);
        }
    }
}

void GameScreen::onMouseClick(sf::Vector2f pos) {
    // Shop tab button
    float shopBtnX = BOARD_X + BOARD_W + 16.f;
    float shopBtnY = INV_BAR_Y;
    if (sf::FloatRect{{shopBtnX, shopBtnY},{INV_BAR_H, INV_BAR_H}}.contains(pos)) {
        shopOverlay_.open(); return;
    }

    // Inventory bar slots
    inventoryOverlay_.handleClick(pos);

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
void GameScreen::selectInventoryItem(const std::string& name) {
    const auto* def = findItem(name);
    if (def && def->type == ShopItemType::SEED) {
        selectedSeed_ = name;
        equippedTool_ = "";
        setStatus("Selected: " + def->cropName + " seed", 1.5f);
    } else if (def && def->type == ShopItemType::TOOL) {
        equippedTool_ = name;
        selectedSeed_ = "";
        setStatus("Equipped: " + def->cropName, 1.5f);
    }
}

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

    session_.save();
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
    if (!back.item_.getMutationList().empty()) ss << " (" << back.item_.getMutations() << ")";
    setStatus(ss.str(), 3.f);

    session_.save();
}

void GameScreen::sellAll() {
    if (harvestBasket_.empty()) { setStatus("Harvest basket is empty!"); return; }
    float total = 0.f;
    for (const auto& e : harvestBasket_) total += (float)e.item_.getPrice();
    harvestBasket_.clear();
    game_.getPlayer().addSheckles(total);
    std::ostringstream ss;
    ss << "Sold for +" << std::fixed << std::setprecision(0) << total << " sheckles!";
    setStatus(ss.str(), 3.f);
    session_.save();
}

void GameScreen::sellOne(int index) {
    if (index < 0 || index >= (int)harvestBasket_.size()) return;
    float price = (float)harvestBasket_[index].item_.getPrice();
    std::string crop = harvestBasket_[index].cropName_;
    harvestBasket_.erase(harvestBasket_.begin() + index);
    game_.getPlayer().addSheckles(price);
    std::ostringstream ss;
    ss << "Sold " << crop << " for +" << std::fixed << std::setprecision(0) << price << " S!";
    setStatus(ss.str(), 2.5f);
    session_.save();
}

void GameScreen::sellGroup(const std::vector<int>& indices) {
    if (indices.empty()) return;

    std::vector<int> sorted = indices;
    std::sort(sorted.begin(), sorted.end(), std::greater<int>());

    float total = 0.f;
    std::string crop = harvestBasket_[sorted.front()].cropName_;
    int soldCount = 0;

    for (int index : sorted) {
        if (index < 0 || index >= static_cast<int>(harvestBasket_.size())) continue;
        total += static_cast<float>(harvestBasket_[index].item_.getPrice());
        harvestBasket_.erase(harvestBasket_.begin() + index);
        ++soldCount;
    }

    if (soldCount == 0) return;

    game_.getPlayer().addSheckles(total);
    std::ostringstream ss;
    ss << "Sold x" << soldCount << " " << crop << " for +"
       << std::fixed << std::setprecision(0) << total << " S!";
    setStatus(ss.str(), 2.5f);
    session_.save();
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
    session_.save();
}

void GameScreen::buyItem(const ShopItemDef& def) {
    if (game_.getPlayer().getSheckles() < def.buyPrice) {
        setStatus("Not enough sheckles! (need " + std::to_string((int)def.buyPrice) + ")");
        return;
    }
    game_.getPlayer().deductSheckles(def.buyPrice);

    if (def.type == ShopItemType::TOOL) {
        if (def.name == "Watering Can") {
            auto item = std::make_unique<WateringCan>();
            game_.getPlayer().getInventory().addItem(std::move(item), 1);
        } else if (def.name == "Fertilizer") {
            auto item = std::make_unique<FertilizerTool>();
            game_.getPlayer().getInventory().addItem(std::move(item), 1);
        }
    } else {
        auto item = std::make_unique<Seed>(1, def.name, def.description,
                                           def.buyPrice, def.cropName);
        game_.getPlayer().getInventory().addItem(std::move(item), 1);
    }

    if (def.type == ShopItemType::SEED) {
        selectedSeed_ = def.name;
        equippedTool_.clear();
    } else {
        equippedTool_ = def.name;
        selectedSeed_.clear();
    }

    setStatus("Bought " + def.cropName + "!", 2.5f);
    session_.save();
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
    if (!inventoryOverlay_.isOpen()) inventoryOverlay_.draw(mouse);
    drawShopTabButton(mouse);
    drawStatus();

    if (shopOverlay_.isOpen()) shopOverlay_.draw(mouse);
    if (inventoryOverlay_.isOpen()) inventoryOverlay_.draw(mouse);

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
