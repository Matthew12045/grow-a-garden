#include "ShopOverlay.h"

#include "GameScreen.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

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
        const std::string key = entry.cropName_ + "|" + makeMutationKey(entry.item_.getMutationList());

        auto it = std::find_if(groups.begin(), groups.end(), [&](const SellGroup& group) {
            return group.cropName == entry.cropName_ && group.mutationKey == key;
        });

        if (it == groups.end()) {
            groups.push_back({entry.cropName_, key, {i}});
        } else {
            it->indices.push_back(i);
        }
    }

    return groups;
}
} // namespace

ShopOverlay::ShopOverlay(sf::RenderWindow& window, sf::Font& font, Game& game,
                         std::vector<BasketEntry>& harvestBasket,
                         const std::vector<ShopItemDef>& catalogue,
                         GameScreen& owner)
    : window_(window),
      font_(font),
      game_(game),
      harvestBasket_(harvestBasket),
      catalogue_(catalogue),
      owner_(owner)
{
}

ShopOverlay::~ShopOverlay() = default;

bool ShopOverlay::isOpen() const {
    return shopOpen_;
}

void ShopOverlay::open(int tab) {
    shopOpen_ = true;
    shopTab_ = tab;
}

void ShopOverlay::close() {
    shopOpen_ = false;
}

void ShopOverlay::draw(sf::Vector2f mouse) {
    sf::RectangleShape dim({1920.f,1080.f});
    dim.setFillColor({0,0,0,190});
    window_.draw(dim);

    DrawUtils::drawPxPanel(window_, font_, {SHOP_X, SHOP_Y},{SHOP_W, SHOP_H},
                Pal::SHOP_BG, {25,14,4}, {75,48,18});

    {
        sf::RectangleShape tbar({SHOP_W, 62.f});
        tbar.setPosition({SHOP_X, SHOP_Y});
        tbar.setFillColor({38,22,8});
        window_.draw(tbar);

        for (float cx : {SHOP_X+24.f}) {
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

        float cx = SHOP_X+SHOP_W-56.f, cy = SHOP_Y+8.f;
        bool  chov = (mouse.x>=cx&&mouse.x<cx+42.f&&mouse.y>=cy&&mouse.y<cy+42.f);
        sf::RectangleShape cb({42.f,42.f});
        cb.setPosition({cx,cy});
        cb.setFillColor(chov ? sf::Color{210,55,55} : sf::Color{150,38,38});
        window_.draw(cb);

        sf::RectangleShape chl({42.f,3.f});
        chl.setPosition({cx,cy});
        chl.setFillColor({255,255,255,50});
        window_.draw(chl);
        auto xb = DrawUtils::makeText(font_, "X", 26, Pal::CREAM);
        xb.setPosition({cx+11.f, cy+7.f});
        window_.draw(xb);
    }

    float tabY = SHOP_Y + 66.f, tabH = 44.f, tabW = 180.f;
    const char* tabs[] = {"SEEDS","TOOLS","SELL"};
    for (int t = 0; t < 3; ++t) {
        float tx   = SHOP_X + 30.f + t*(tabW+10.f);
        bool  act  = (shopTab_==t);
        bool  thov = (mouse.x>=tx&&mouse.x<tx+tabW&&mouse.y>=tabY&&mouse.y<tabY+tabH);
        sf::Color tc = act ? Pal::TAB_ACT : (thov ? sf::Color{130,82,35} : Pal::TAB_INACT);
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

    sf::RectangleShape div({SHOP_W-60.f,2.f});
    div.setPosition({SHOP_X+30.f, SHOP_Y+114.f});
    div.setFillColor({75,48,18});
    window_.draw(div);

    float cardsY = SHOP_Y + 122.f;
    float cardsH = SHOP_H - 122.f - 14.f;
    float cardsX = SHOP_X + 22.f;
    float cardsW = SHOP_W - 44.f;

    if (shopTab_ == 0) {
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

void ShopOverlay::drawSeedCard(const ShopItemDef& def, sf::FloatRect b, sf::Vector2f mouse) {
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

    float iconSz = b.size.y * 0.52f;
    float iconX  = b.position.x + b.size.x * 0.5f;
    float iconY  = b.position.y + iconSz * 0.56f;

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

void ShopOverlay::drawToolCard(const ShopItemDef& def, sf::FloatRect b, sf::Vector2f mouse) {
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

void ShopOverlay::drawSellPage(sf::Vector2f mouse) {
    float cardsY = SHOP_Y + 122.f;
    float cardsH = SHOP_H - 122.f - 14.f;
    float cardsX = SHOP_X + 22.f;
    float cardsW = SHOP_W - 44.f;
    const auto groups = groupHarvestBasket(harvestBasket_);

    if (groups.empty()) {
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

    float headerH = 52.f;
    sf::RectangleShape hdr({cardsW, headerH});
    hdr.setPosition({cardsX, cardsY});
    hdr.setFillColor({30,18,6});
    window_.draw(hdr);

    sf::RectangleShape hdrHl({cardsW, 3.f});
    hdrHl.setPosition({cardsX, cardsY});
    hdrHl.setFillColor({80,55,20});
    window_.draw(hdrHl);

    float total = 0.f;
    for (const auto& e : harvestBasket_) total += (float)e.item_.getPrice();

    auto totalTxt = DrawUtils::makeText(font_, "Basket: " + std::to_string((int)harvestBasket_.size()) +
                             " items  |  Total: " +
                             [&]{ std::ostringstream s;
                                  s << std::fixed << std::setprecision(0) << total;
                                  return s.str(); }() + " S",
                             22, Pal::CREAM);
    totalTxt.setPosition({cardsX + 16.f, cardsY + 14.f});
    window_.draw(totalTxt);

    float saW = 160.f, saH = 36.f;
    sf::FloatRect saBtn{{cardsX + cardsW - saW - 10.f, cardsY + 8.f},{saW, saH}};
    DrawUtils::drawPxButton(window_, font_, saBtn, "SELL ALL",
                 {170, 110, 20}, {210, 140, 25}, mouse, 20);

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
        float price = (float)entry.item_.getPrice();
        bool  hasMut = !entry.item_.getMutationList().empty();
        int   count = static_cast<int>(group.indices.size());

        sf::RectangleShape bg(b.size);
        bg.setPosition(b.position);
        bg.setFillColor(hov ? Pal::CARD_HOV : Pal::CARD_BG);
        window_.draw(bg);

        sf::RectangleShape border(b.size);
        border.setPosition(b.position);
        border.setFillColor(sf::Color::Transparent);
        border.setOutlineThickness(2.f);
        border.setOutlineColor(hasMut ? Pal::MUTATION
                               : (hov ? Pal::GOLD : sf::Color{75,48,18}));
        window_.draw(border);

        float iconSz = cardH * 0.72f;
        float iconCX = b.position.x + iconSz * 0.56f;
        float iconCY = b.position.y + cardH * 0.46f;
        sf::RectangleShape iconBg({iconSz, iconSz});
        iconBg.setPosition({b.position.x, b.position.y + (cardH - iconSz)/2.f});
        iconBg.setFillColor({28,16,5});
        window_.draw(iconBg);

        const auto* def = findItemByCrop(entry.cropName_);
        if (def) CropRenderer::drawCropIcon(window_, font_, *def, {iconCX, iconCY}, iconSz * 0.70f);

        float tx = b.position.x + iconSz + 10.f;
        float tw = cardW - iconSz - 14.f;
        float ty = b.position.y + 12.f;

        auto nameT = DrawUtils::makeText(font_, entry.cropName_, 20, hov ? Pal::GOLD : Pal::CREAM);
        nameT.setPosition({tx, ty});
        window_.draw(nameT);

        if (count > 1) {
            auto countT = DrawUtils::makeText(font_, "x" + std::to_string(count), 15, Pal::GOLD);
            countT.setPosition({tx + tw - 24.f, ty - 2.f});
            window_.draw(countT);
        }

        std::ostringstream ps;
        ps << std::fixed << std::setprecision(0) << price << " S";
        auto priceT = DrawUtils::makeText(font_, ps.str(), 18, {150, 210, 130});
        priceT.setPosition({tx, ty + 26.f});
        window_.draw(priceT);

        if (hasMut) {
            auto mutT = DrawUtils::makeText(font_, entry.item_.getMutations(), 13, Pal::MUTATION);
            mutT.setPosition({tx, ty + 50.f});
            window_.draw(mutT);
        }

        sf::FloatRect btnR = getSellButtonRect(b, iconSz);
        DrawUtils::drawPxButton(window_, font_, btnR, count > 1 ? ("SELL x" + std::to_string(count)) : "SELL",
                     {170, 110, 20}, {215, 145, 28},
                     mouse, 16);
    }
}

void ShopOverlay::handleClick(sf::Vector2f pos) {
    float cx=SHOP_X+SHOP_W-56.f, cy=SHOP_Y+8.f;
    if (pos.x>=cx&&pos.x<cx+42.f&&pos.y>=cy&&pos.y<cy+42.f) {
        shopOpen_=false; return;
    }

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
        std::vector<const ShopItemDef*> seeds;
        for (const auto& d:catalogue_) if(d.type==ShopItemType::SEED) seeds.push_back(&d);
        int cols=4, n=(int)seeds.size(), rows=(n+cols-1)/cols;
        float cw=(cardsW-(cols-1)*10.f)/cols;
        float ch=std::min(175.f,(cardsH-(rows-1)*10.f)/rows);
        for (int i=0;i<n;++i) {
            sf::FloatRect cardR = getSeedCardRect(i, cols, cardsX, cardsY, cw, ch);
            sf::FloatRect btnR = getSeedButtonRect(cardR);
            if (btnR.contains(pos)) { owner_.buyItem(*seeds[i]); return; }
        }
    } else if (shopTab_==1) {
        std::vector<const ShopItemDef*> tools;
        for (const auto& d:catalogue_) if(d.type==ShopItemType::TOOL) tools.push_back(&d);
        float cw=(cardsW-20.f)/2.f, ch=std::min(260.f,cardsH);
        for (int i=0;i<(int)tools.size();++i) {
            sf::FloatRect cardR = getToolCardRect(i, cardsX, cardsY, cw, ch);
            sf::FloatRect btnR = getToolButtonRect(cardR);
            if (btnR.contains(pos)) { owner_.buyItem(*tools[i]); return; }
        }
    } else {
        const auto groups = groupHarvestBasket(harvestBasket_);
        if (groups.empty()) return;

        float headerH = 52.f;

        float saW=160.f, saH=36.f;
        sf::FloatRect saBtn{{cardsX+cardsW-saW-10.f, cardsY+8.f},{saW,saH}};
        if (saBtn.contains(pos)) { owner_.sellAll(); return; }

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
            if (btnR.contains(pos)) { owner_.sellGroup(groups[i].indices); return; }
        }
    }
}

sf::FloatRect ShopOverlay::getSeedCardRect(int i, int cols, float cardsX, float cardsY, float cardW, float cardH) const {
    int col = i % cols;
    int row = i / cols;
    return sf::FloatRect{{cardsX + col * (cardW + 10.f), cardsY + row * (cardH + 10.f)}, {cardW, cardH}};
}

sf::FloatRect ShopOverlay::getSeedButtonRect(sf::FloatRect cardRect) const {
    constexpr float btnW = 68.f;
    constexpr float btnH = 26.f;
    float footerY = cardRect.position.y + cardRect.size.y - 34.f;
    return sf::FloatRect{{cardRect.position.x + cardRect.size.x - btnW - 6.f, footerY + 4.f}, {btnW, btnH}};
}

sf::FloatRect ShopOverlay::getToolCardRect(int i, float cardsX, float cardsY, float cardW, float cardH) const {
    return sf::FloatRect{{cardsX + i * (cardW + 20.f), cardsY + 20.f}, {cardW, cardH}};
}

sf::FloatRect ShopOverlay::getToolButtonRect(sf::FloatRect cardRect) const {
    constexpr float btnW = 110.f;
    constexpr float btnH = 34.f;
    float footerY = cardRect.position.y + cardRect.size.y - 48.f;
    return sf::FloatRect{{cardRect.position.x + cardRect.size.x - btnW - 10.f, footerY - 4.f}, {btnW, btnH}};
}

sf::FloatRect ShopOverlay::getSellCardRect(int i, int cols, float cardsX, float gridY, float cardW, float cardH) const {
    int col = i % cols;
    int row = i / cols;
    return sf::FloatRect{{cardsX + col * (cardW + 10.f), gridY + row * (cardH + 10.f)}, {cardW, cardH}};
}

sf::FloatRect ShopOverlay::getSellButtonRect(sf::FloatRect cardRect, float iconSz) const {
    float tx = cardRect.position.x + iconSz + 10.f;
    float btnW = cardRect.size.x - iconSz - 14.f;
    constexpr float btnH = 26.f;
    return sf::FloatRect{{tx, cardRect.position.y + cardRect.size.y - btnH - 6.f}, {btnW, btnH}};
}

const ShopItemDef* ShopOverlay::findItemByCrop(const std::string& cropName) const {
    for (const auto& d : catalogue_)
        if (d.cropName == cropName) return &d;
    return nullptr;
}
