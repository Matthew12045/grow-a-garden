#ifndef TITLE_SCREEN_APP_H
#define TITLE_SCREEN_APP_H

#include <SFML/Graphics.hpp>
#include "TitleScreen.h"
#include "../ui/AudioManager.h"
#include <memory>

class TitleScreenApp {
public:
    TitleScreenApp();
    ~TitleScreenApp() = default;

    int run();

private:
    std::unique_ptr<sf::RenderWindow> window;
    std::unique_ptr<TitleScreen> titleScreen;
    sf::Font font;
    AudioManager audioManager_;
    
    void loadAssets();
    void handleEvents();
    void runGame();       // ← launches the main gameplay loop
};

#endif
