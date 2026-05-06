#include "CreditsScreen.h"
#include <cmath>

CreditsScreen::CreditsScreen()
    : visible(false), returnToTitle(false), scrollOffset(0.f), 
      lineSpacing(42.f), scrollSpeed(30.f), fontPtr(nullptr), dimAlpha(0.f) {
}

void CreditsScreen::setFont(const sf::Font& font) {
    fontPtr = &font;
}

void CreditsScreen::show() {
    visible = true;
    returnToTitle = false;
    scrollOffset = 0.f;
    dimAlpha = 0.f;  // Start with no dimming
}

void CreditsScreen::hide() {
    visible = false;
    returnToTitle = false;  // Reset the flag when hiding
}

void CreditsScreen::goBack() {
    returnToTitle = true;
}

void CreditsScreen::update(float deltaTime) {
    if (!visible) return;
    
    // Gradually dim to black (but cap at 150 so text stays visible)
    dimAlpha = std::min(150.f, dimAlpha + 85.f * deltaTime);
    
    // Scroll upward
    scrollOffset += scrollSpeed * deltaTime;
    
    // Auto return after all credits scroll past
    float totalHeight = credits.size() * lineSpacing;
    if (scrollOffset > totalHeight + 800.f) {
        goBack();
    }
}

bool isAllCaps(const std::string& str) {
    if (str.empty()) return false;
    for (char c : str) {
        if (std::isalpha(c) && !std::isupper(c)) {
            return false;
        }
    }
    return true;
}

void CreditsScreen::draw(sf::RenderWindow& window) {
    if (!visible || !fontPtr) return;
    
    // Don't draw black background - let title background show through and dim
    
    float windowWidth = window.getSize().x;
    float windowHeight = window.getSize().y;
    float fadeZoneHeight = windowHeight * 0.15f; // 15% at top and bottom fade
    
    // Draw dim overlay FIRST (behind the text)
    sf::RectangleShape dimOverlay(sf::Vector2f(windowWidth, windowHeight));
    dimOverlay.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(dimAlpha)));
    window.draw(dimOverlay);
    
    // Draw each credit line (on top, not darkened)
    for (size_t i = 0; i < credits.size(); ++i) {
        const std::string& text = credits[i];
        if (text.empty()) continue; // Skip empty lines
        
        // Determine size: ALL CAPS = title (larger), else name (smaller)
        unsigned int fontSize = isAllCaps(text) ? 32 : 24;
        sf::Text creditText(*fontPtr, text, fontSize);
        
        // Center horizontally
        auto bounds = creditText.getLocalBounds();
        float centerX = windowWidth / 2.f - bounds.size.x / 2.f;
        
        // Calculate Y: start from bottom, move up with scroll
        float lineY = windowHeight + (i * lineSpacing) - scrollOffset;
        creditText.setPosition(sf::Vector2f(centerX, lineY));
        
        // Calculate alpha fade
        float alpha = 255.f;
        if (lineY < fadeZoneHeight) {
            // Top fade zone
            alpha = (lineY / fadeZoneHeight) * 255.f;
        } else if (lineY > windowHeight - fadeZoneHeight) {
            // Bottom fade zone
            alpha = ((windowHeight - lineY) / fadeZoneHeight) * 255.f;
        }
        
        // Clamp and apply color
        alpha = std::max(0.f, std::min(255.f, alpha));
        sf::Color color(200, 200, 200, static_cast<std::uint8_t>(alpha));
        creditText.setFillColor(color);
        
        window.draw(creditText);
    }
    
    // Draw "Press ESC to Return" at top
    sf::Text escText(*fontPtr, "Press ESC to Return", 16);
    escText.setFillColor(sf::Color(150, 150, 150, 200));
    auto bounds = escText.getLocalBounds();
    escText.setPosition(sf::Vector2f(windowWidth / 2.f - bounds.size.x / 2.f, 20.f));
    window.draw(escText);
}
