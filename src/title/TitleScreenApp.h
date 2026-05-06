#ifndef TITLE_SCREEN_APP_H
#define TITLE_SCREEN_APP_H

#include <SFML/Graphics.hpp>
#include "TitleScreen.h"
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
    
    void loadAssets();
    void handleEvents();
};

#endif
