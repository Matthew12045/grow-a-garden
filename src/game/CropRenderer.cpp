#include "CropRenderer.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace CropRenderer {

namespace {

enum class CropAssetRole {
    Sprout,
    Plant,
    Harvested
};

struct CropVisualAssets {
    const char* sproutPath;
    const char* plantPath;
    const char* harvestedPath;
};

struct CachedCropAsset {
    sf::Texture texture;
    sf::IntRect sourceRect;
    bool attempted = false;
    bool loaded = false;
};

const CropVisualAssets* findCropVisualAssets(const std::string& cropName) {
    static const std::unordered_map<std::string, CropVisualAssets> cropAssets = {
        {"Carrot", {
            "assets/textures/plants/carrot_sprout.png",
            "assets/textures/plants/carrot_plant.png",
            "assets/textures/plants/carrot_harvested.png",
        }},
    };

    const auto it = cropAssets.find(cropName);
    return it == cropAssets.end() ? nullptr : &it->second;
}

const char* getAssetPath(const CropVisualAssets& assets, CropAssetRole role) {
    switch (role) {
        case CropAssetRole::Sprout:    return assets.sproutPath;
        case CropAssetRole::Plant:     return assets.plantPath;
        case CropAssetRole::Harvested: return assets.harvestedPath;
    }

    return nullptr;
}

sf::IntRect findOpaqueBounds(const sf::Image& image) {
    const sf::Vector2u size = image.getSize();
    if (size.x == 0 || size.y == 0) {
        return sf::IntRect{{0, 0}, {0, 0}};
    }

    unsigned int minX = size.x;
    unsigned int minY = size.y;
    unsigned int maxX = 0;
    unsigned int maxY = 0;
    bool foundOpaquePixel = false;

    for (unsigned int y = 0; y < size.y; ++y) {
        for (unsigned int x = 0; x < size.x; ++x) {
            if (image.getPixel({x, y}).a == 0) {
                continue;
            }

            foundOpaquePixel = true;
            minX = std::min(minX, x);
            minY = std::min(minY, y);
            maxX = std::max(maxX, x);
            maxY = std::max(maxY, y);
        }
    }

    if (!foundOpaquePixel) {
        return sf::IntRect{{0, 0}, {static_cast<int>(size.x), static_cast<int>(size.y)}};
    }

    return sf::IntRect{
        {static_cast<int>(minX), static_cast<int>(minY)},
        {static_cast<int>(maxX - minX + 1), static_cast<int>(maxY - minY + 1)}
    };
}

const CachedCropAsset* getCachedAsset(const std::string& path) {
    static std::unordered_map<std::string, CachedCropAsset> cache;

    CachedCropAsset& asset = cache[path];
    if (!asset.attempted) {
        asset.attempted = true;

        sf::Image image;
        if (image.loadFromFile(path) && asset.texture.loadFromImage(image)) {
            asset.texture.setSmooth(false);
            asset.sourceRect = findOpaqueBounds(image);
            asset.loaded = asset.sourceRect.size.x > 0 && asset.sourceRect.size.y > 0;
        }
    }

    return asset.loaded ? &asset : nullptr;
}

bool drawCroppedAsset(sf::RenderWindow& window, const std::string& path,
                      sf::Vector2f centre, float targetMaxSize) {
    const CachedCropAsset* asset = getCachedAsset(path);
    if (!asset || targetMaxSize <= 0.f) {
        return false;
    }

    sf::Sprite sprite(asset->texture);
    sprite.setTextureRect(asset->sourceRect);

    const sf::FloatRect bounds = sprite.getLocalBounds();
    const float maxSourceSize = std::max(bounds.size.x, bounds.size.y);
    if (maxSourceSize <= 0.f) {
        return false;
    }

    const float scale = targetMaxSize / maxSourceSize;
    sprite.setOrigin({bounds.position.x + bounds.size.x / 2.f,
                      bounds.position.y + bounds.size.y / 2.f});
    sprite.setScale({scale, scale});
    sprite.setPosition(centre);
    window.draw(sprite);
    return true;
}

bool drawCropAsset(sf::RenderWindow& window, const std::string& cropName,
                   CropAssetRole role, sf::Vector2f centre, float targetMaxSize) {
    const CropVisualAssets* assets = findCropVisualAssets(cropName);
    if (!assets) {
        return false;
    }

    const char* path = getAssetPath(*assets, role);
    return path != nullptr && drawCroppedAsset(window, path, centre, targetMaxSize);
}

} // namespace

void drawProceduralCropIcon(sf::RenderWindow& window, sf::Font& font,
                            const ShopItemDef& def, sf::Vector2f c, float sz);

void drawCloud(sf::RenderWindow& window, float cx, float cy, float scale) {
    auto circle = [&](float ox, float oy, float r) {
        sf::CircleShape sh(r);
        sh.setFillColor(Pal::CLOUD);
        sh.setPosition({cx + ox - r, cy + oy - r});
        window.draw(sh);
    };
    float r = 38.f * scale;
    circle(0, 0, r);
    circle(r * 1.1f, -r * 0.5f, r * 1.35f);
    circle(r * 2.3f, 0, r);
}

void drawPlant(sf::RenderWindow& window, sf::Font& font, Plant* p, sf::Vector2f s,
               float cellSz, const std::vector<ShopItemDef>& catalogue) {
    if (!p) return;
    int   stage    = p->getStage();
    int   maxStage = p->getMaxStages();
    bool  ripe     = p->isFullyGrown();
    float ratio    = maxStage > 0 ? (float)stage / (float)maxStage : 0.f;
    float cx = s.x + cellSz / 2.f;
    float cy = s.y + cellSz / 2.f;

    if (drawCropAsset(window, p->getName(),
                      ripe ? CropAssetRole::Plant : CropAssetRole::Sprout,
                      {cx, cy + cellSz * 0.03f}, cellSz * 0.66f)) {
        return;
    }

    if (ripe) {
        std::string seedKey = "Carrot Seed";
        for (const auto& d : catalogue)
            if (d.cropName == p->getName()) { seedKey = d.name; break; }

        const ShopItemDef* def = nullptr;
        for (const auto& d : catalogue)
            if (d.name == seedKey) { def = &d; break; }

        if (def) drawProceduralCropIcon(window, font, *def, {cx, cy}, 70.f);
    } else {
        sf::Color stemCol = Pal::SPROUT;
        for (const auto& d : catalogue)
            if (d.cropName == p->getName()) { stemCol = d.iconAccent; break; }

        float h = 10.f + ratio * (cellSz * 0.38f);
        sf::RectangleShape stem({5.f, h});
        stem.setPosition({cx - 2.5f, cy + 8.f - h});
        stem.setFillColor(stemCol);
        window.draw(stem);
        if (stage >= 1) {
            float lw = 8.f + ratio * 28.f;
            sf::RectangleShape leaf({lw, 5.f});
            leaf.setPosition({cx - lw / 2.f, cy + 6.f - h});
            leaf.setFillColor(stemCol);
            window.draw(leaf);
        }
        if (stage >= 2) {
            float lw2 = 6.f + ratio * 20.f;
            sf::RectangleShape leaf2({lw2, 4.f});
            leaf2.setPosition({cx - lw2 / 2.f, cy - h * 0.4f});
            leaf2.setFillColor(stemCol);
            window.draw(leaf2);
        }
    }
}

void drawCropIcon(sf::RenderWindow& window, sf::Font& font, const ShopItemDef& def, sf::Vector2f c, float sz) {
    if (drawCropAsset(window, def.cropName, CropAssetRole::Harvested, c, sz)) {
        return;
    }

    drawProceduralCropIcon(window, font, def, c, sz);
}

void drawProceduralCropIcon(sf::RenderWindow& window, sf::Font&, const ShopItemDef& def, sf::Vector2f c, float sz) {
    sf::Color col  = def.iconPrimary;
    sf::Color stem = def.iconAccent;
    float h = sz * 0.55f;
    float w = sz * 0.40f;

    auto rect = [&](float ox, float oy, float rw, float rh, sf::Color rc) {
        sf::RectangleShape r({rw, rh});
        r.setPosition({c.x + ox - rw / 2.f, c.y + oy - rh / 2.f});
        r.setFillColor(rc);
        window.draw(r);
    };
    auto circle = [&](float ox, float oy, float r, sf::Color rc) {
        sf::CircleShape sh(r);
        sh.setFillColor(rc);
        sh.setPosition({c.x + ox - r, c.y + oy - r});
        window.draw(sh);
    };
    auto tri = [&](float x0, float y0, float x1, float y1, float x2, float y2, sf::Color rc) {
        sf::ConvexShape sh(3);
        sh.setPoint(0, {c.x + x0, c.y + y0});
        sh.setPoint(1, {c.x + x1, c.y + y1});
        sh.setPoint(2, {c.x + x2, c.y + y2});
        sh.setFillColor(rc);
        window.draw(sh);
    };

    const std::string& n = def.cropName;

    if (n == "Carrot") {
        tri(-w / 2, -h / 2 + sz * 0.1f, w / 2, -h / 2 + sz * 0.1f, 0, h / 2, col);
        rect(0, -h / 2 - sz * 0.1f, sz * 0.06f, sz * 0.2f, stem);
        rect(-sz * 0.12f, -h / 2 - sz * 0.12f, sz * 0.08f, sz * 0.16f, stem);
        rect( sz * 0.12f, -h / 2 - sz * 0.12f, sz * 0.08f, sz * 0.16f, stem);
    }
    else if (n == "Blueberry") {
        circle(0, 0, sz * 0.18f, col);
        circle(-sz * 0.18f, -sz * 0.12f, sz * 0.15f, col);
        circle( sz * 0.18f, -sz * 0.12f, sz * 0.15f, col);
        circle(-sz * 0.12f,  sz * 0.18f, sz * 0.13f, col);
        circle( sz * 0.12f,  sz * 0.18f, sz * 0.13f, col);
        rect(0, -h / 2 - sz * 0.05f, sz * 0.06f, sz * 0.15f, stem);
    }
    else if (n == "Rose") {
        sf::ConvexShape diamond(4);
        diamond.setPoint(0, {c.x,      c.y - h / 2});
        diamond.setPoint(1, {c.x + w / 2,  c.y});
        diamond.setPoint(2, {c.x,      c.y + h / 2 * 0.6f});
        diamond.setPoint(3, {c.x - w / 2,  c.y});
        diamond.setFillColor(col);
        window.draw(diamond);
        rect(0, h / 2 * 0.6f + sz * 0.1f, sz * 0.07f, sz * 0.25f, stem);
        rect(-sz * 0.12f, h / 2 * 0.1f, sz * 0.12f, sz * 0.06f, stem);
    }
    else if (n == "Bamboo") {
        for (int i = -1; i <= 1; ++i) {
            float ox = i * sz * 0.18f;
            rect(ox, 0, sz * 0.10f, h * 0.9f, col);
            for (float jy = -0.3f; jy <= 0.3f; jy += 0.3f)
                rect(ox, jy * h, sz * 0.14f, sz * 0.04f, stem);
        }
    }
    else if (n == "Corn") {
        rect(0, sz * 0.05f, sz * 0.22f, h * 0.75f, col);
        rect(-sz * 0.18f, 0, sz * 0.12f, h * 0.5f, stem);
        rect( sz * 0.18f, 0, sz * 0.12f, h * 0.5f, stem);
        rect(0, -h / 2 - sz * 0.05f, sz * 0.07f, sz * 0.18f, stem);
    }
    else if (n == "Tomato") {
        circle(0, sz * 0.06f, sz * 0.30f, col);
        rect(0, -sz * 0.22f, sz * 0.06f, sz * 0.16f, stem);
        rect(-sz * 0.10f, -sz * 0.18f, sz * 0.12f, sz * 0.06f, stem);
        rect( sz * 0.10f, -sz * 0.18f, sz * 0.12f, sz * 0.06f, stem);
    }
    else if (n == "Apple") {
        circle(0, sz * 0.06f, sz * 0.28f, col);
        rect(0, -sz * 0.24f, sz * 0.05f, sz * 0.14f, {120,70,30});
        rect(sz * 0.06f, -sz * 0.24f, sz * 0.12f, sz * 0.06f, stem);
        circle(0, -sz * 0.22f, sz * 0.06f, Pal::BOARD_DRK);
    }
    else if (n == "Cactus") {
        rect(0, 0, sz * 0.20f, h * 0.95f, col);
        rect(0, -sz * 0.05f, h * 0.65f, sz * 0.18f, col);
        for (int i = -1; i <= 1; i += 2) {
            rect(sz * i * 0.18f, -sz * 0.15f, sz * 0.04f, sz * 0.08f, Pal::CREAM);
            rect(sz * i * 0.18f,  sz * 0.10f, sz * 0.04f, sz * 0.08f, Pal::CREAM);
        }
    }
    else if (n == "Coconut") {
        circle(0, sz * 0.04f, sz * 0.30f, col);
        circle(0, sz * 0.04f, sz * 0.18f, {90,50,20});
        rect(0, -sz * 0.30f, sz * 0.06f, sz * 0.14f, stem);
    }
    else if (n == "Beanstalk") {
        for (int i = 0; i < 4; ++i) {
            float yo = -h / 2 + i * h / 3.5f;
            float xo = (i % 2 == 0) ? -sz * 0.12f : sz * 0.12f;
            circle(xo, yo, sz * 0.10f, col);
        }
        rect(0, 0, sz * 0.05f, h, stem);
        rect(-sz * 0.18f, -h * 0.2f, sz * 0.20f, sz * 0.07f, stem);
    }
    else if (n == "Cacao") {
        sf::ConvexShape pod(8);
        float pw = sz * 0.22f, ph = h * 0.80f;
        pod.setPoint(0, {c.x,           c.y - ph});
        pod.setPoint(1, {c.x + pw * 0.6f,   c.y - ph * 0.5f});
        pod.setPoint(2, {c.x + pw,         c.y});
        pod.setPoint(3, {c.x + pw * 0.6f,   c.y + ph * 0.5f});
        pod.setPoint(4, {c.x,            c.y + ph});
        pod.setPoint(5, {c.x - pw * 0.6f,   c.y + ph * 0.5f});
        pod.setPoint(6, {c.x - pw,         c.y});
        pod.setPoint(7, {c.x - pw * 0.6f,   c.y - ph * 0.5f});
        pod.setFillColor(col);
        window.draw(pod);
        for (int i = -1; i <= 1; ++i)
            rect(i * pw * 0.4f, 0, sz * 0.03f, ph * 1.6f,
                 {(uint8_t)(col.r + 20), (uint8_t)(col.g + 15), (uint8_t)(col.b + 10)});
        rect(0, -ph - sz * 0.08f, sz * 0.05f, sz * 0.14f, stem);
    }
    else if (n == "Watering Can") {
        sf::ConvexShape body(4);
        body.setPoint(0, {c.x - w * 0.8f, c.y - h * 0.3f});
        body.setPoint(1, {c.x + w * 0.5f, c.y - h * 0.3f});
        body.setPoint(2, {c.x + w * 0.7f, c.y + h * 0.4f});
        body.setPoint(3, {c.x - w * 0.8f, c.y + h * 0.4f});
        body.setFillColor(col);
        window.draw(body);
        rect(w * 0.65f, -h * 0.05f, sz * 0.30f, sz * 0.07f, col);
        rect(w * 0.15f, -h * 0.45f, sz * 0.07f, sz * 0.20f, stem);
        rect(w * 0.08f, -h * 0.38f, sz * 0.20f, sz * 0.06f, stem);
    }
    else {
        sf::ConvexShape bag(6);
        bag.setPoint(0, {c.x - w * 0.3f, c.y - h * 0.6f});
        bag.setPoint(1, {c.x + w * 0.3f, c.y - h * 0.6f});
        bag.setPoint(2, {c.x + w * 0.7f, c.y - h * 0.1f});
        bag.setPoint(3, {c.x + w * 0.6f, c.y + h * 0.6f});
        bag.setPoint(4, {c.x - w * 0.6f, c.y + h * 0.6f});
        bag.setPoint(5, {c.x - w * 0.7f, c.y - h * 0.1f});
        bag.setFillColor(col);
        window.draw(bag);
        rect(0, 0, sz * 0.22f, sz * 0.06f, stem);
        rect(0, -sz * 0.08f, sz * 0.06f, sz * 0.20f, stem);
    }
}

}
