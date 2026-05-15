#ifndef BUTTON_H
#define BUTTON_H

#include <SFML/Graphics.hpp>
#include <string>
#include <functional>
#include <memory>

class Button {
public:
    enum class State {
        Normal,
        Hovered,
        Pressed
    };

    Button(float x, float y, float width, float height, const std::string& text);
    ~Button() = default;

    void setFont(const sf::Font& font);
    void setTexture(const sf::Texture& texture);
    void update(const sf::Vector2f& mousePos);
    void update(const sf::Vector2f& mousePos, float deltaTime);
    void draw(sf::RenderWindow& window);
    
    bool isHovered() const { return state == State::Hovered; }
    bool isPressed() const { return state == State::Pressed; }
    void setPressed(bool pressed);
    
    const sf::FloatRect& getBounds() const { return bounds; }
    void setPosition(float x, float y);
    sf::Vector2f getPosition() const { return bounds.position; }
    
    void setOnClickCallback(std::function<void()> callback) { onClickCallback = callback; }

private:
    sf::ConvexShape buttonShape;
    std::unique_ptr<sf::Text> buttonText;
    std::unique_ptr<sf::Sprite> buttonSprite;
    const sf::Texture* buttonTexture;
    sf::FloatRect bounds;
    State state;
        std::string buttonTextString;
    std::function<void()> onClickCallback;
    const sf::Font* fontPtr;
    
    sf::Color normalColor;
    sf::Color hoveredColor;
    sf::Color pressedColor;
    sf::Color textNormalColor;
    sf::Color textHoveredColor;
    
    void updateColors();
    float currentScale;
    float targetScale;
    void updateScale(float deltaTime);
    bool drawRectangle;
    void drawFallbackText(sf::RenderWindow& window, const std::string& text, const sf::Vector2f& pos, const sf::Vector2f& size, const sf::Color& color);
};

#endif
