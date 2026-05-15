#ifndef TITLE_SCREEN_H
#define TITLE_SCREEN_H

#include <SFML/Graphics.hpp>
#include "Button.h"
#include "CreditsScreen.h"
#include <memory>

class TitleScreen {
public:
    enum class State {
        FadingIn,
        IntroHoldLogo,
        IntroDimLogo,
        IntroLoading,
        Active,
        Transitioning,
        Credits,
        ConfirmStart,
        ConfirmReset
    };

    TitleScreen(const std::string& backgroundPath);
    ~TitleScreen() = default;

    void setFont(const sf::Font& font);
    void update(float deltaTime, const sf::Vector2f& mousePos, bool mousePressed);
    void draw(sf::RenderWindow& window);
    
    bool shouldStartGame() const { return shouldStart; }
    bool shouldExit() const { return shouldExitGame; }
    
    void reset();
    
    State getCurrentState() const { return currentState; }

private:
    State currentState;
    
    sf::Texture backgroundTexture;
    std::unique_ptr<sf::Sprite> backgroundSprite;
    
    sf::Texture introTexture;
    std::unique_ptr<sf::Sprite> introSprite;
    
    sf::Texture startTexture;
    sf::Texture resetTexture;
    sf::Texture leaveTexture;
    
    std::unique_ptr<Button> startButton;
    std::unique_ptr<Button> creditsButton;
    std::unique_ptr<Button> exitButton;
    std::unique_ptr<Button> resetButton;
    
    std::unique_ptr<CreditsScreen> creditsScreen;
    std::unique_ptr<Button> confirmYesButton;
    std::unique_ptr<Button> confirmNoButton;
    std::unique_ptr<sf::Text> confirmText;
    
    float fadeAlpha;
    float maxFade;
    float fadeSpeed;
    float introTimer;
    float loadingTimer;
    float loadingDuration;
    
    // Intro timing constants
    static constexpr float kIntroFadeDuration = 2.0f;
    static constexpr float kIntroHoldDuration = 2.0f;
    static constexpr float kIntroDimDuration = 1.5f;
    static constexpr float kLoadingDuration = 3.0f;
    
    bool shouldStart;
    bool shouldExitGame;
    bool shouldResetGame;
    bool exitButtonEvading;
    sf::Vector2f exitButtonHomePos;
    sf::Vector2f exitButtonTargetPos;
    sf::Vector2f exitButtonVelocity;
    float evasionTimer;
    
    void updateExitButtonEvasion(const sf::Vector2f& mousePos, float deltaTime);
    void handleButtonClicks();
    void resetGame();
};

#endif
