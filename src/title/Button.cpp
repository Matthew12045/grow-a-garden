#include "Button.h"
#include <cmath>
#include <unordered_map>
#include <array>

Button::Button(float x, float y, float width, float height, const std::string& text)
        : bounds(sf::Vector2f(x, y), sf::Vector2f(width, height)), state(State::Normal),
            buttonText(nullptr),
    fontPtr(nullptr),
        normalColor(30, 90, 200, 255),
        hoveredColor(60, 130, 240, 255),
        pressedColor(20, 60, 160, 255),
        textNormalColor(255, 255, 255),
        textHoveredColor(255, 220, 0) {
    
    // Create rounded rectangle shape
    // Use rectangle (4 points) for consistent appearance
    buttonShape.setPointCount(4);
    float radius = 10.f;
    float angle = 0.f;
    // define rectangle points
    buttonShape.setPoint(0, sf::Vector2f(0.f, 0.f));
    buttonShape.setPoint(1, sf::Vector2f(width, 0.f));
    buttonShape.setPoint(2, sf::Vector2f(width, height));
    buttonShape.setPoint(3, sf::Vector2f(0.f, height));
    
    buttonShape.setPosition(sf::Vector2f(x, y));
    buttonShape.setFillColor(normalColor);
    buttonShape.setOutlineColor(sf::Color(0,0,0,100));
    buttonShape.setOutlineThickness(2.f);

    // Use text-only buttons by default (no rectangle drawn)
    drawRectangle = false;

    buttonTextString = text;
    buttonText = nullptr;
    buttonSprite = nullptr;
    buttonTexture = nullptr;
    currentScale = 1.0f;
    targetScale = 1.0f;
}

Button::~Button() {
    if (buttonText) delete buttonText;
    if (buttonSprite) delete buttonSprite;
}

void Button::setFont(const sf::Font& font) {
    fontPtr = &font;
    if (buttonText) delete buttonText;
    // Create text and center it within the button bounds
    // Use a 1.5x initial font size for better visibility
    unsigned int baseSize = 36;
    unsigned int size = static_cast<unsigned int>(std::round(baseSize * 1.5f));
    buttonText = new sf::Text(font, buttonTextString, size);
    buttonText->setFillColor(textNormalColor);
    auto tb = buttonText->getLocalBounds();
    buttonText->setPosition(sf::Vector2f(
        bounds.position.x + bounds.size.x / 2.f - tb.size.x / 2.f,
        bounds.position.y + bounds.size.y / 2.f - tb.size.y / 2.f - 5.f
    ));
    buttonText->setScale(sf::Vector2f(currentScale, currentScale));
}

void Button::setTexture(const sf::Texture& texture) {
    buttonTexture = &texture;
    if (buttonSprite) delete buttonSprite;
    buttonSprite = new sf::Sprite(texture);
    
    // Set origin to the center of the image to scale from the center
    auto texSize = texture.getSize();
    buttonSprite->setOrigin(sf::Vector2f(texSize.x / 2.f, texSize.y / 2.f));
    
    float baseScaleX = bounds.size.x / (float)texSize.x;
    float baseScaleY = bounds.size.y / (float)texSize.y;
    
    buttonSprite->setPosition(sf::Vector2f(
        bounds.position.x + bounds.size.x / 2.f,
        bounds.position.y + bounds.size.y / 2.f
    ));
    buttonSprite->setScale(sf::Vector2f(baseScaleX * currentScale, baseScaleY * currentScale));
    
    // Initial color
    updateColors();
}

void Button::update(const sf::Vector2f& mousePos) {
    update(mousePos, 0.0f);
}

void Button::update(const sf::Vector2f& mousePos, float deltaTime) {
    bool wasHovered = (state == State::Hovered);

    if (bounds.contains(mousePos)) {
        state = State::Hovered;
        targetScale = 2.0f; // double size on hover
    } else {
        if (state != State::Pressed) {
            state = State::Normal;
        }
        targetScale = 1.0f;
    }

    updateColors();
    updateScale(deltaTime);
}

void Button::draw(sf::RenderWindow& window) {
    if (buttonSprite) {
        window.draw(*buttonSprite);
    } 
    // Text-only button/fallback:
    else if (buttonText) {
        window.draw(*buttonText);
    } else {
        // Draw a simple fallback bitmap text when no font is loaded
        drawFallbackText(window, buttonTextString, bounds.position, bounds.size, textNormalColor);
    }
}

void Button::drawFallbackText(sf::RenderWindow& window, const std::string& text, const sf::Vector2f& pos, const sf::Vector2f& size, const sf::Color& color) {
    // Simple 5x7 pixel font for uppercase letters used in our labels.
    static std::unordered_map<char, std::array<uint8_t,7>> glyphs;
    if (glyphs.empty()) {
        glyphs['A'] = {0b01110,0b10001,0b10001,0b11111,0b10001,0b10001,0b10001};
        glyphs['C'] = {0b01110,0b10001,0b10000,0b10000,0b10000,0b10001,0b01110};
        glyphs['D'] = {0b11110,0b10001,0b10001,0b10001,0b10001,0b10001,0b11110};
        glyphs['E'] = {0b11111,0b10000,0b10000,0b11110,0b10000,0b10000,0b11111};
        glyphs['I'] = {0b11111,0b00100,0b00100,0b00100,0b00100,0b00100,0b11111};
        glyphs['L'] = {0b10000,0b10000,0b10000,0b10000,0b10000,0b10000,0b11111};
        glyphs['R'] = {0b11110,0b10001,0b10001,0b11110,0b10100,0b10010,0b10001};
        glyphs['S'] = {0b01111,0b10000,0b10000,0b01110,0b00001,0b00001,0b11110};
        glyphs['T'] = {0b11111,0b00100,0b00100,0b00100,0b00100,0b00100,0b00100};
        glyphs['V'] = {0b10001,0b10001,0b10001,0b10001,0b10001,0b01010,0b00100};
    }

    float glyphW = 5.f;
    float glyphH = 7.f;
    float spacing = 1.f;
    size_t len = text.size();
    float totalW = len * (glyphW + spacing);

    float pixelSizeX = std::floor(size.x / (totalW + 2.f));
    float pixelSizeY = std::floor(size.y / (glyphH + 2.f));
    float pixel = std::max(1.f, std::min(pixelSizeX, pixelSizeY));

    float startX = pos.x + (size.x - (len * (glyphW + spacing) * pixel)) / 2.f;
    float startY = pos.y + (size.y - (glyphH * pixel)) / 2.f;

    sf::RectangleShape rect(sf::Vector2f(pixel, pixel));
    rect.setFillColor(color);

    for (size_t i = 0; i < len; ++i) {
        char c = text[i];
        if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
        auto it = glyphs.find(c);
        if (it == glyphs.end()) continue;
        const auto& rows = it->second;
        for (int y = 0; y < 7; ++y) {
            uint8_t row = rows[y];
            for (int x = 0; x < 5; ++x) {
                if (row & (1 << (4 - x))) {
                    rect.setPosition(sf::Vector2f(startX + (i * (glyphW + spacing) + x) * pixel, startY + y * pixel));
                    window.draw(rect);
                }
            }
        }
    }
}

void Button::setPressed(bool pressed) {
    state = pressed ? State::Pressed : State::Normal;
    updateColors();
}

void Button::setPosition(float x, float y) {
    buttonShape.setPosition(sf::Vector2f(x, y));
    bounds.position = sf::Vector2f(x, y);
    
    if (buttonSprite) {
        buttonSprite->setPosition(sf::Vector2f(
            x + bounds.size.x / 2.f,
            y + bounds.size.y / 2.f
        ));
    }
    
    if (buttonText) {
        auto textBounds = buttonText->getLocalBounds();
        buttonText->setPosition(sf::Vector2f(
            x + bounds.size.x / 2.f - textBounds.size.x / 2.f,
            y + bounds.size.y / 2.f - textBounds.size.y / 2.f - 5.f
        ));
    }
}

void Button::updateColors() {
    switch (state) {
        case State::Normal:
            if (buttonText) buttonText->setFillColor(textNormalColor);
            if (buttonSprite) buttonSprite->setColor(sf::Color(180, 180, 180, 255)); // darker
            break;
        case State::Hovered:
            if (buttonText) buttonText->setFillColor(textHoveredColor);
            if (buttonSprite) buttonSprite->setColor(sf::Color(255, 255, 255, 255)); // light up
            break;
        case State::Pressed:
            if (buttonText) buttonText->setFillColor(textHoveredColor);
            if (buttonSprite) buttonSprite->setColor(sf::Color(255, 255, 255, 255));
            break;
    }
}

void Button::updateScale(float deltaTime) {
    // Smoothly interpolate currentScale -> targetScale
    const float stiffness = 12.f; // higher = snappier
    float t = 1.f - std::exp(-stiffness * std::max(0.0f, deltaTime));
    currentScale += (targetScale - currentScale) * t;
    
    if (buttonSprite) {
        auto texSize = buttonTexture->getSize();
        float baseScaleX = bounds.size.x / (float)texSize.x;
        float baseScaleY = bounds.size.y / (float)texSize.y;
        // make it grow nicely by not modifying bounds position but just the internal scale
        // scale limit so it doesn't get ridiculously large, just slightly pop out like targetScale (up to 2.0x for text, maybe 1.15x for image)
        float imageScale = 1.0f + (currentScale - 1.0f) * 0.15f; 
        buttonSprite->setScale(sf::Vector2f(baseScaleX * imageScale, baseScaleY * imageScale));
    }
    
    // Apply scale to text and recenter
    if (buttonText) {
        buttonText->setScale(sf::Vector2f(currentScale, currentScale));
        auto tb = buttonText->getLocalBounds();
        float scaledW = tb.size.x * currentScale;
        float scaledH = tb.size.y * currentScale;
        buttonText->setPosition(sf::Vector2f(
            bounds.position.x + bounds.size.x / 2.f - scaledW / 2.f,
            bounds.position.y + bounds.size.y / 2.f - scaledH / 2.f - 5.f
        ));
        // Update text color handled in updateColors
    }
}
