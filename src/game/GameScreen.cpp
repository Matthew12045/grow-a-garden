#include "GameScreen.h"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <iostream>

// ─────────────────────────────────────────────────────────────────────
// Constructor / setup
// ─────────────────────────────────────────────────────────────────────
GameScreen::GameScreen(sf::RenderWindow& window, sf::Font& font)
    : window_(window), font_(font), game_(), shop_()
{
    // 2 ticks/sec  →  carrot (5 stages × 10 ticks) ≈ 25 s to grow
    game_.getTickSystem().setTickRate(0.5f);

    // Starting resources
    game_.getPlayer().addSheckles(100.0f);
    auto seed = std::make_unique<Seed>(1, "Carrot Seed",
                                       "Grows into a carrot.", 10.0, "Carrot");
    game_.getPlayer().getInventory().addItem(std::move(seed), 5);

    setupShop();
}

void GameScreen::setupShop() {
    shop_.addAvailableItem(
        std::make_unique<Seed>(1, "Carrot Seed",
                               "Grows into a carrot.", 10.0, "Carrot"));
}

// ─────────────────────────────────────────────────────────────────────
// Main loop
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
// Events
// ─────────────────────────────────────────────────────────────────────
void GameScreen::handleEvent(const sf::Event& ev) {
    if (ev.is<sf::Event::Closed>()) { window_.close(); return; }

    if (auto* kp = ev.getIf<sf::Event::KeyPressed>()) {
        if (kp->code == sf::Keyboard::Key::Escape) window_.close();
        // F = fast-forward toggle
        if (kp->code == sf::Keyboard::Key::F) {
            fastMode_ = !fastMode_;
            game_.getTickSystem().setTickRate(fastMode_ ? 0.05f : 0.5f);
            setStatus(fastMode_ ? "Fast mode ON" : "Normal speed", 1.5f);
        }
    }

    if (auto* mb = ev.getIf<sf::Event::MouseButtonReleased>()) {
        if (mb->button == sf::Mouse::Button::Left)
            onMouseClick({ (float)mb->position.x, (float)mb->position.y });
    }
}

void GameScreen::onMouseClick(sf::Vector2f pos) {
    // ── Garden grid ──────────────────────────────────────────────────
    bool inGrid = (pos.x >= GRID_X && pos.x < GRID_X + 20*CELL_SZ &&
                   pos.y >= GRID_Y && pos.y < GRID_Y + 20*CELL_SZ);
    if (inGrid) { handleCellClick(pos); return; }

    // ── Shop button ──────────────────────────────────────────────────
    float shopX = BOARD_X + BOARD_W + 16.f;
    float shopY = INV_BAR_Y;
    sf::FloatRect shopRect{{ shopX, shopY }, { INV_BAR_H, INV_BAR_H }};
    if (shopRect.contains(pos)) { buySeed(); return; }

    // ── Inventory sell slot (slot 1 – harvested items) ────────────────
    float slotX = BOARD_X;
    float slotY = INV_BAR_Y + (INV_BAR_H - INV_SLOT_SZ) / 2.f;
    for (int i = 0; i < INV_SLOTS; ++i) {
        sf::FloatRect sr{{ slotX + i*(INV_SLOT_SZ+6.f), slotY },
                          { INV_SLOT_SZ, INV_SLOT_SZ }};
        if (sr.contains(pos)) {
            // Slot 0 = seeds slot  →  buy seed
            // Slot 1 = harvest slot → sell all
            if (i == 0) buySeed();
            else if (i == 1 && !harvestBasket_.empty()) sellAll();
            return;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────
// Update
// ─────────────────────────────────────────────────────────────────────
void GameScreen::update(float dt) {
    game_.update(dt);

    std::size_t newTick = game_.getTickSystem().getTick();
    if (newTick > lastTick_) {
        std::size_t delta = newTick - lastTick_;
        lastTick_ = newTick;

        // Advance day counter every 60 ticks
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
// Actions
// ─────────────────────────────────────────────────────────────────────
void GameScreen::handleCellClick(sf::Vector2f pos) {
    int gx = (int)((pos.x - GRID_X) / CELL_SZ);
    int gy = (int)((pos.y - GRID_Y) / CELL_SZ);
    if (gx < 0 || gx >= 20 || gy < 0 || gy >= 20) return;

    Cell&  cell  = game_.getGarden().getCell(gx, gy);
    Plant* plant = cell.getPlant();

    if (plant) {
        if (plant->isFullyGrown()) harvestCell(gx, gy);
        else setStatus("Growing... " + std::to_string(plant->getTimeToGrowth()) + " ticks left.");
    } else {
        if (game_.getPlayer().getInventory().getQuantity("Carrot Seed") > 0)
            plantSeed(gx, gy);
        else
            setStatus("No seeds!  Buy some from the shop.");
    }
}

void GameScreen::plantSeed(int gx, int gy) {
    game_.getPlayer().getInventory().removeItem("Carrot Seed", 1);
    game_.getGarden().plantCrop(gx, gy, std::make_unique<Carrot>());
    setStatus("Planted a carrot seed!");
}

void GameScreen::harvestCell(int gx, int gy) {
    Cell&  cell  = game_.getGarden().getCell(gx, gy);
    Plant* plant = cell.getPlant();
    if (!plant || !plant->isFullyGrown()) return;

    HarvestedItem item = plant->harvest();
    float price = (float)item.getPrice();
    harvestBasket_.push_back(item);

    if (plant->isFullyGrown()) cell.clearPlant();   // consumed → remove

    std::ostringstream ss;
    ss << "Harvested!  Worth " << std::fixed << std::setprecision(0) << price << " sheckles";
    if (!item.getMutationList().empty()) ss << "  (" << item.getMutations() << ")";
    setStatus(ss.str(), 3.f);
}

void GameScreen::buySeed() {
    const float PRICE = 10.f;
    if (game_.getPlayer().getSheckles() < PRICE) {
        setStatus("Not enough sheckles! (need 10)");
        return;
    }
    game_.getPlayer().deductSheckles(PRICE);
    auto seed = std::make_unique<Seed>(1, "Carrot Seed", "Grows into a carrot.", PRICE, "Carrot");
    game_.getPlayer().getInventory().addItem(std::move(seed), 1);
    setStatus("Bought a Carrot Seed!");
}

void GameScreen::sellAll() {
    if (harvestBasket_.empty()) { setStatus("Harvest basket empty!"); return; }
    float total = 0.f;
    for (const auto& item : harvestBasket_) total += (float)item.getPrice();
    harvestBasket_.clear();
    game_.getPlayer().addSheckles(total);
    std::ostringstream ss;
    ss << "Sold for " << std::fixed << std::setprecision(0) << total << " sheckles!";
    setStatus(ss.str(), 3.f);
}

void GameScreen::setStatus(const std::string& msg, float dur) {
    statusMsg_   = msg;
    statusTimer_ = dur;
}

// ─────────────────────────────────────────────────────────────────────
// Rendering  –  master
// ─────────────────────────────────────────────────────────────────────
void GameScreen::render() {
    sf::Vector2f mouse = window_.mapPixelToCoords(sf::Mouse::getPosition(window_));
    window_.clear(Pal::SKY);

    drawBackground();
    drawTopUI();
    drawGardenBoard(mouse);
    drawInventoryBar(mouse);
    drawShopButton(mouse);
    drawStatus();

    window_.display();
}

// ─────────────────────────────────────────────────────────────────────
// Background: sky + clouds
// ─────────────────────────────────────────────────────────────────────
void GameScreen::drawBackground() {
    // Sky already cleared.  Draw gradient bands for depth.
    {
        sf::RectangleShape top({ 1920.f, 180.f });
        top.setFillColor({ 88, 162, 210 });
        window_.draw(top);
    }
    {
        sf::RectangleShape bot({ 1920.f, 200.f });
        bot.setPosition({ 0.f, 880.f });
        bot.setFillColor({ 98, 178, 215 });
        window_.draw(bot);
    }

    // Clouds
    const CloudDef clouds[] = {
        {130.f,  85.f, 1.00f},
        {370.f,  48.f, 0.72f},
        {1145.f, 65.f, 1.18f},
        {1545.f,105.f, 0.88f},
        {778.f,  38.f, 0.62f},
        {1350.f, 40.f, 0.55f},
    };
    for (auto& c : clouds) drawCloud(c.x, c.y, c.scale);
}

void GameScreen::drawCloud(float cx, float cy, float scale) {
    // A cloud = 3 overlapping circles
    auto circle = [&](float offX, float offY, float r) {
        sf::CircleShape sh(r);
        sh.setFillColor(Pal::CLOUD);
        sh.setPosition({ cx + offX - r, cy + offY - r });
        window_.draw(sh);
    };
    float r = 38.f * scale;
    circle(0,       0,      r);
    circle(r * 1.1f,-r*0.5f, r * 1.35f);
    circle(r * 2.3f, 0,      r);
}

// ─────────────────────────────────────────────────────────────────────
// Pixel-art panel helper
// ─────────────────────────────────────────────────────────────────────
void GameScreen::drawPxPanel(sf::Vector2f pos, sf::Vector2f size) {
    // Outer dark border
    {
        sf::RectangleShape s(size + sf::Vector2f(6.f, 6.f));
        s.setPosition(pos - sf::Vector2f(3.f, 3.f));
        s.setFillColor(Pal::FRAME_DRK);
        window_.draw(s);
    }
    // Main body
    {
        sf::RectangleShape s(size);
        s.setPosition(pos);
        s.setFillColor(Pal::FRAME_MID);
        window_.draw(s);
    }
    // Top-left highlight strip
    {
        sf::RectangleShape top({ size.x, 4.f });
        top.setPosition(pos);
        top.setFillColor(Pal::FRAME_LIT);
        window_.draw(top);

        sf::RectangleShape left({ 4.f, size.y });
        left.setPosition(pos);
        left.setFillColor(Pal::FRAME_LIT);
        window_.draw(left);
    }
    // Bottom-right shadow strip
    {
        sf::RectangleShape bot({ size.x, 4.f });
        bot.setPosition({ pos.x, pos.y + size.y - 4.f });
        bot.setFillColor(Pal::BOARD_DRK);
        window_.draw(bot);

        sf::RectangleShape right({ 4.f, size.y });
        right.setPosition({ pos.x + size.x - 4.f, pos.y });
        right.setFillColor(Pal::BOARD_DRK);
        window_.draw(right);
    }
}

// ─────────────────────────────────────────────────────────────────────
// Top UI:  Day counter (left) + Sheckles (right)
// ─────────────────────────────────────────────────────────────────────
void GameScreen::drawTopUI() {
    // ── Day frame ────────────────────────────────────────────────────
    {
        sf::Vector2f pos{ 18.f, 14.f };
        sf::Vector2f sz { 210.f, 54.f };
        drawPxPanel(pos, sz);

        // Little sun icon (circle)
        sf::CircleShape sun(14.f);
        sun.setFillColor({ 255, 210, 40 });
        sun.setPosition({ 30.f, 22.f });
        window_.draw(sun);

        // Day text
        std::string dayStr = "DAY " + std::to_string(dayCount_);
        auto t = makeText(dayStr, 24, Pal::DARKTEXT);
        auto lb = t.getLocalBounds();
        t.setPosition({ pos.x + sz.x/2.f - lb.size.x/2.f + 12.f, pos.y + sz.y/2.f - lb.size.y/2.f });
        window_.draw(t);
    }

    // ── Sheckles frame ───────────────────────────────────────────────
    {
        sf::Vector2f sz{ 220.f, 54.f };
        sf::Vector2f pos{ 1920.f - sz.x - 18.f, 14.f };
        drawPxPanel(pos, sz);

        // Coin icon (circle)
        sf::CircleShape coin(14.f);
        coin.setFillColor(Pal::GOLD);
        coin.setPosition({ pos.x + 14.f, pos.y + 13.f });
        window_.draw(coin);

        // Dollar sign inside coin
        auto cText = makeText("$", 16, Pal::DARKTEXT);
        cText.setPosition({ pos.x + 20.f, pos.y + 15.f });
        window_.draw(cText);

        // Amount
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(0)
           << game_.getPlayer().getSheckles() << " S";
        auto t = makeText(ss.str(), 26, Pal::GOLD);
        auto lb = t.getLocalBounds();
        t.setPosition({ pos.x + sz.x - lb.size.x - 16.f, pos.y + sz.y/2.f - lb.size.y/2.f });
        window_.draw(t);
    }

    // ── Weather strip (top centre) ───────────────────────────────────
    {
        WeatherType wt = game_.getWeatherSystem().getCurrentWeather();
        static const char* WX[] = {"Summer","Rain","Frost","Thunder","Meteors"};
        int wi = (int)wt;
        std::string wStr = std::string(WX[wi]) + "  (F=fast)";

        sf::Vector2f sz{ 300.f, 40.f };
        sf::Vector2f pos{ 960.f - sz.x/2.f, 18.f };
        drawPxPanel(pos, sz);

        auto t = makeText(wStr, 20, Pal::DARKTEXT);
        auto lb = t.getLocalBounds();
        t.setPosition({ pos.x + sz.x/2.f - lb.size.x/2.f, pos.y + sz.y/2.f - lb.size.y/2.f });
        window_.draw(t);
    }
}

// ─────────────────────────────────────────────────────────────────────
// Garden board
// ─────────────────────────────────────────────────────────────────────
void GameScreen::drawGardenBoard(sf::Vector2f mouse) {
    // ── Outer dark drop-shadow ────────────────────────────────────────
    {
        sf::RectangleShape shadow({ BOARD_W + 8.f, BOARD_H + 8.f });
        shadow.setPosition({ BOARD_X - 2.f, BOARD_Y + 4.f });
        shadow.setFillColor({ 30, 14, 4, 160 });
        window_.draw(shadow);
    }

    // ── Main board (dark outer border) ───────────────────────────────
    {
        sf::RectangleShape border({ BOARD_W + 6.f, BOARD_H + 6.f });
        border.setPosition({ BOARD_X - 3.f, BOARD_Y - 3.f });
        border.setFillColor(Pal::BOARD_DRK);
        window_.draw(border);
    }

    // ── Board face ───────────────────────────────────────────────────
    {
        sf::RectangleShape face({ BOARD_W, BOARD_H });
        face.setPosition({ BOARD_X, BOARD_Y });
        face.setFillColor(Pal::BOARD_MID);
        window_.draw(face);
    }

    // Board highlight (top + left inner edge)
    {
        sf::RectangleShape top({ BOARD_W, BOARD_PAD });
        top.setPosition({ BOARD_X, BOARD_Y });
        top.setFillColor(Pal::BOARD_LIT);
        window_.draw(top);

        sf::RectangleShape left({ BOARD_PAD, BOARD_H });
        left.setPosition({ BOARD_X, BOARD_Y });
        left.setFillColor(Pal::BOARD_LIT);
        window_.draw(left);
    }

    // Board shadow (bottom + right inner edge)
    {
        sf::RectangleShape bot({ BOARD_W, BOARD_PAD });
        bot.setPosition({ BOARD_X, BOARD_Y + BOARD_H - BOARD_PAD });
        bot.setFillColor(Pal::BOARD_DRK);
        window_.draw(bot);

        sf::RectangleShape right({ BOARD_PAD, BOARD_H });
        right.setPosition({ BOARD_X + BOARD_W - BOARD_PAD, BOARD_Y });
        right.setFillColor(Pal::BOARD_DRK);
        window_.draw(right);
    }

    // ── Grid cells ───────────────────────────────────────────────────
    const Garden& garden = game_.getGarden();
    for (int gy = 0; gy < garden.getHeight(); ++gy) {
        for (int gx = 0; gx < garden.getWidth(); ++gx) {
            sf::Vector2f screen {
                GRID_X + gx * CELL_SZ,
                GRID_Y + gy * CELL_SZ
            };
            bool hov = (mouse.x >= screen.x && mouse.x < screen.x + CELL_SZ &&
                        mouse.y >= screen.y && mouse.y < screen.y + CELL_SZ);
            drawCell(gx, gy, screen, hov);
        }
    }

    // ── Hint text below board ────────────────────────────────────────
    auto hint = makeText("Click empty soil = plant  |  Click glowing cell = harvest", 17, { 60, 35, 12 });
    hint.setPosition({ BOARD_X + 10.f, BOARD_Y + BOARD_H + 4.f });
    window_.draw(hint);
}

// ─────────────────────────────────────────────────────────────────────
// Single cell
// ─────────────────────────────────────────────────────────────────────
void GameScreen::drawCell(int gx, int gy, sf::Vector2f s, bool hov) {
    const float GAP = 1.5f;
    const float SZ  = CELL_SZ - GAP;

    const Cell& cell  = game_.getGarden().getCell(gx, gy);
    Plant*      plant = cell.getPlant();

    sf::Color base = plant ? Pal::SOIL_DRK : Pal::SOIL_MID;
    if (hov && !plant) base = { 160, 110, 56 };
    if (hov && plant && plant->isFullyGrown()) base = { 200, 180, 40 };

    // Shadow (bottom-right)
    {
        sf::RectangleShape sh({ SZ, SZ });
        sh.setPosition({ s.x + GAP, s.y + GAP });
        sh.setFillColor(Pal::SOIL_DRK);
        window_.draw(sh);
    }
    // Face
    {
        sf::RectangleShape face({ SZ - 2.f, SZ - 2.f });
        face.setPosition({ s.x + GAP, s.y + GAP });
        face.setFillColor(base);
        window_.draw(face);
    }
    // Highlight (top-left pixel lines)
    {
        sf::RectangleShape hl({ SZ - 2.f, 2.f });
        hl.setPosition({ s.x + GAP, s.y + GAP });
        hl.setFillColor(Pal::SOIL_LIT);
        window_.draw(hl);

        sf::RectangleShape vl({ 2.f, SZ - 2.f });
        vl.setPosition({ s.x + GAP, s.y + GAP });
        vl.setFillColor(Pal::SOIL_LIT);
        window_.draw(vl);
    }

    if (plant) drawPlant(plant, s);
}

// ─────────────────────────────────────────────────────────────────────
// Plant drawing (geometric pixel-art shapes, no sprites)
// ─────────────────────────────────────────────────────────────────────
void GameScreen::drawPlant(Plant* p, sf::Vector2f s) {
    if (!p) return;

    int stage    = p->getStage();
    int maxStage = p->getMaxStages();
    bool ripe    = p->isFullyGrown();

    float cx = s.x + CELL_SZ / 2.f;
    float cy = s.y + CELL_SZ / 2.f;

    if (ripe) {
        // ── Ripe carrot: orange body + green top ──────────────────────
        // Body (triangle pointing down = carrot shape)
        sf::ConvexShape carrot(3);
        float hw = 7.f, ht = 12.f;
        carrot.setPoint(0, { cx - hw, cy - 4.f });
        carrot.setPoint(1, { cx + hw, cy - 4.f });
        carrot.setPoint(2, { cx,      cy + ht  });
        carrot.setFillColor({ 230, 120, 30 });
        window_.draw(carrot);

        // Green stem (rectangle above body)
        sf::RectangleShape stem({ 4.f, 6.f });
        stem.setPosition({ cx - 2.f, cy - 10.f });
        stem.setFillColor({ 60, 170, 60 });
        window_.draw(stem);

        // Leaves (two small diagonal rects)
        sf::RectangleShape lLeaf({ 6.f, 3.f });
        lLeaf.setPosition({ cx - 8.f, cy - 11.f });
        lLeaf.setFillColor({ 60, 170, 60 });
        lLeaf.setRotation(sf::degrees(-30.f));
        window_.draw(lLeaf);

        sf::RectangleShape rLeaf({ 6.f, 3.f });
        rLeaf.setPosition({ cx + 2.f, cy - 11.f });
        rLeaf.setFillColor({ 60, 170, 60 });
        rLeaf.setRotation(sf::degrees(30.f));
        window_.draw(rLeaf);

        // Glow ring around cell when ripe
        sf::RectangleShape glow({ CELL_SZ - 3.f, CELL_SZ - 3.f });
        glow.setPosition({ s.x + 1.5f, s.y + 1.5f });
        glow.setFillColor(sf::Color::Transparent);
        glow.setOutlineThickness(2.f);
        glow.setOutlineColor({ 255, 220, 30, 200 });
        window_.draw(glow);

    } else {
        // ── Growing stages: green sprout ─────────────────────────────
        float ratio  = (float)stage / (float)maxStage;
        float height = 4.f + ratio * 12.f;

        // Stem
        sf::RectangleShape stem({ 3.f, height });
        stem.setPosition({ cx - 1.5f, cy + 4.f - height });
        stem.setFillColor(Pal::SPROUT);
        window_.draw(stem);

        // Leaf (small horizontal bar at top)
        if (stage >= 1) {
            float lw = 3.f + ratio * 7.f;
            sf::RectangleShape leaf({ lw, 3.f });
            leaf.setPosition({ cx - lw/2.f, cy + 3.f - height });
            leaf.setFillColor({ 50, 150, 50 });
            window_.draw(leaf);
        }
    }

    // Mutation dot (top-right corner)
    if (!p->getMutations().empty()) {
        sf::CircleShape dot(3.5f);
        dot.setFillColor(Pal::MUTATION);
        dot.setPosition({ s.x + CELL_SZ - 8.f, s.y + 2.f });
        window_.draw(dot);
    }
}

// ─────────────────────────────────────────────────────────────────────
// Inventory bar (bottom)
// ─────────────────────────────────────────────────────────────────────
void GameScreen::drawInventoryBar(sf::Vector2f mouse) {
    // Background panel spanning below the board
    float barX = BOARD_X;
    float barY = INV_BAR_Y;
    float barW = BOARD_W;

    drawPxPanel({ barX, barY }, { barW, INV_BAR_H });

    // Slots
    float slotY  = barY + (INV_BAR_H - INV_SLOT_SZ) / 2.f;
    int   seeds  = game_.getPlayer().getInventory().getQuantity("Carrot Seed");
    int   harvests = (int)harvestBasket_.size();

    for (int i = 0; i < INV_SLOTS; ++i) {
        float slotX = barX + 6.f + i * (INV_SLOT_SZ + 5.f);
        bool  hov   = mouse.x >= slotX && mouse.x < slotX + INV_SLOT_SZ &&
                      mouse.y >= slotY && mouse.y < slotY + INV_SLOT_SZ;

        // Slot background (3D sunken look)
        {
            sf::RectangleShape shadow({ INV_SLOT_SZ, INV_SLOT_SZ });
            shadow.setPosition({ slotX, slotY });
            shadow.setFillColor(Pal::SOIL_DRK);
            window_.draw(shadow);

            sf::Color face = hov ? sf::Color{ 155, 108, 55 } : Pal::SOIL_MID;
            sf::RectangleShape f({ INV_SLOT_SZ - 3.f, INV_SLOT_SZ - 3.f });
            f.setPosition({ slotX, slotY });
            f.setFillColor(face);
            window_.draw(f);
        }

        // Slot content
        if (i == 0 && seeds > 0) {
            // Seed icon: tiny orange carrot  
            sf::ConvexShape seed(3);
            float cx = slotX + INV_SLOT_SZ/2.f;
            float cy = slotY + INV_SLOT_SZ/2.f + 2.f;
            seed.setPoint(0, { cx-5.f, cy-6.f });
            seed.setPoint(1, { cx+5.f, cy-6.f });
            seed.setPoint(2, { cx,     cy+8.f });
            seed.setFillColor({ 220, 110, 25 });
            window_.draw(seed);

            sf::RectangleShape stem({ 3.f, 5.f });
            stem.setPosition({ cx-1.5f, cy-11.f });
            stem.setFillColor(Pal::SPROUT);
            window_.draw(stem);

            // Count badge
            auto cnt = makeText(std::to_string(seeds), 15, Pal::GOLD);
            cnt.setPosition({ slotX + INV_SLOT_SZ - 18.f, slotY + INV_SLOT_SZ - 20.f });
            window_.draw(cnt);

        } else if (i == 1 && harvests > 0) {
            // Harvest icon: stack of orange carrots
            float cx = slotX + INV_SLOT_SZ/2.f;
            float cy = slotY + INV_SLOT_SZ/2.f + 4.f;

            sf::ConvexShape hv(3);
            hv.setPoint(0, { cx-7.f, cy-8.f });
            hv.setPoint(1, { cx+7.f, cy-8.f });
            hv.setPoint(2, { cx,     cy+10.f });
            hv.setFillColor({ 220, 110, 25 });
            window_.draw(hv);

            sf::RectangleShape stems({ 4.f, 6.f });
            stems.setPosition({ cx-2.f, cy-14.f });
            stems.setFillColor(Pal::SPROUT);
            window_.draw(stems);

            auto cnt = makeText(std::to_string(harvests), 15, Pal::GOLD);
            cnt.setPosition({ slotX + INV_SLOT_SZ - 18.f, slotY + INV_SLOT_SZ - 20.f });
            window_.draw(cnt);

            // Tooltip hint
            if (hov) {
                auto tip = makeText("Click to SELL ALL", 16, Pal::GOLD);
                tip.setPosition({ slotX - 20.f, slotY - 24.f });
                window_.draw(tip);
            }
        }

        // Hover outline
        if (hov) {
            sf::RectangleShape outline({ INV_SLOT_SZ - 3.f, INV_SLOT_SZ - 3.f });
            outline.setPosition({ slotX, slotY });
            outline.setFillColor(sf::Color::Transparent);
            outline.setOutlineThickness(2.f);
            outline.setOutlineColor(Pal::GOLD);
            window_.draw(outline);
        }
    }

    // Label above slot 0
    auto l0 = makeText("Seeds", 14, { 60, 36, 12 });
    l0.setPosition({ BOARD_X + 6.f, barY - 18.f });
    window_.draw(l0);

    auto l1 = makeText("Harvest", 14, { 60, 36, 12 });
    l1.setPosition({ BOARD_X + 6.f + INV_SLOT_SZ + 5.f, barY - 18.f });
    window_.draw(l1);
}

// ─────────────────────────────────────────────────────────────────────
// Shop placeholder button (bottom right, beside inventory bar)
// ─────────────────────────────────────────────────────────────────────
void GameScreen::drawShopButton(sf::Vector2f mouse) {
    float shopX = BOARD_X + BOARD_W + 16.f;
    float shopY = INV_BAR_Y;
    float shopSz = INV_BAR_H;

    bool hov = (mouse.x >= shopX && mouse.x < shopX + shopSz &&
                mouse.y >= shopY && mouse.y < shopY + shopSz);

    drawPxPanel({ shopX, shopY }, { shopSz, shopSz });

    // Icon: stack of 3 mini rects (shop shelves look)
    for (int i = 0; i < 3; ++i) {
        sf::RectangleShape shelf({ shopSz - 20.f, 8.f });
        shelf.setPosition({ shopX + 10.f, shopY + 12.f + i * 16.f });
        shelf.setFillColor(hov ? Pal::GOLD : Pal::CREAM);
        window_.draw(shelf);
    }

    auto label = makeText("SHOP", 14, hov ? Pal::GOLD : Pal::DARKTEXT);
    label.setPosition({ shopX + shopSz/2.f - 16.f, shopY + shopSz - 20.f });
    window_.draw(label);

    // Buy seed cost tooltip
    if (hov) {
        auto tip = makeText("Buy Seed -10S", 16, Pal::GOLD);
        tip.setPosition({ shopX - 60.f, shopY - 28.f });
        window_.draw(tip);
    }
}

// ─────────────────────────────────────────────────────────────────────
// Status message  (semi-transparent strip above inventory bar)
// ─────────────────────────────────────────────────────────────────────
void GameScreen::drawStatus() {
    if (statusTimer_ <= 0.f) return;

    float alpha = std::min(1.f, statusTimer_ / 0.4f);
    sf::Color bg{ 30, 14, 4, (uint8_t)(alpha * 210) };
    sf::Color fg{ 255, 228, 100, (uint8_t)(alpha * 255) };

    auto t = makeText(statusMsg_, 22, fg);
    auto lb = t.getLocalBounds();
    float tw = lb.size.x + 32.f;
    float tx = 960.f - tw / 2.f;
    float ty = INV_BAR_Y - 36.f;

    sf::RectangleShape pill({ tw, 30.f });
    pill.setPosition({ tx, ty });
    pill.setFillColor(bg);
    window_.draw(pill);

    t.setPosition({ tx + 16.f, ty + 4.f });
    window_.draw(t);
}

// ─────────────────────────────────────────────────────────────────────
// Text helper
// ─────────────────────────────────────────────────────────────────────
sf::Text GameScreen::makeText(const std::string& s, unsigned sz, sf::Color col) const {
    sf::Text t(font_, s, sz);
    t.setFillColor(col);
    return t;
}

void GameScreen::centreText(sf::Text& t, sf::FloatRect area) {
    auto lb = t.getLocalBounds();
    t.setPosition({
        area.position.x + (area.size.x - lb.size.x) / 2.f,
        area.position.y + (area.size.y - lb.size.y) / 2.f - 2.f
    });
}