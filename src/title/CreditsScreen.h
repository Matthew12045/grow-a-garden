#ifndef CREDITS_SCREEN_H
#define CREDITS_SCREEN_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class CreditsScreen {
public:
    CreditsScreen();
    ~CreditsScreen() = default;

    void setFont(const sf::Font& font);
    void update(float deltaTime);
    void draw(sf::RenderWindow& window);
    
    void show();
    void hide();
    bool isVisible() const { return visible; }
    
    void goBack();
    bool shouldReturnToTitle() const { return returnToTitle; }

private:
    bool visible;
    bool returnToTitle;
    
    const sf::Font* fontPtr;
    float scrollOffset;
    float lineSpacing;
    float scrollSpeed;
    float dimAlpha;      // Darkness level (0-255)
    
    // GTA-style credits: edit this array
    std::vector<std::string> credits = {
        "GROW A GARDEN",
        "",
        "MORE THAN HALF OF FRA143 SCORE",
        "",
        "Made by",
        "MATTHEW",
        "",
        "ART",
        "BUA",
        "",
        "DESIGN",
        "MATTHEW",
        "",
        "SPECIAL THANKS",
        "MATTHEW",
        "",
        "PROMPT ENGINEER",
        "TIAN ZHOU",
        ""
    };
};

#endif
