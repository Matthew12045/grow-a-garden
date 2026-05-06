#include "TitleScreenApp.h"
#include <iostream>

TitleScreenApp::TitleScreenApp() {
    // Create window
    window = std::make_unique<sf::RenderWindow>(
        sf::VideoMode(sf::Vector2u(1920, 1080)),
        "Grow a Garden",
        sf::Style::Default
    );
    window->setFramerateLimit(60);
    
    loadAssets();
    
    // Create title screen
    titleScreen = std::make_unique<TitleScreen>("assets/textures/title_background.png");
    titleScreen->setFont(font);
}

void TitleScreenApp::loadAssets() {
    // Load font
    std::vector<std::string> fontCandidates = {
        "assets/fonts/Arial.ttf",
        "./assets/fonts/Arial.ttf",
        "../assets/fonts/Arial.ttf"
    };

    // Try to use Windows system fonts if available
    const char* windir = getenv("WINDIR");
    if (windir) {
        std::string winFonts = std::string(windir) + "\\Fonts\\";
        fontCandidates.push_back(winFonts + "arial.ttf");
        fontCandidates.push_back(winFonts + "ARIAL.TTF");
        fontCandidates.push_back(winFonts + "DejaVuSans.ttf");
        fontCandidates.push_back(winFonts + "dejavusans.ttf");
    }

    bool loaded = false;
    for (const auto& path : fontCandidates) {
        if (font.openFromFile(path)) {
            std::cout << "Loaded font from: " << path << std::endl;
            loaded = true;
            break;
        }
    }

    if (!loaded) {
        std::cerr << "Warning: Could not load font from assets/fonts/Arial.ttf or system fonts." << std::endl;
        std::cerr << "Text will use fallback bitmap rendering until a TTF is placed in assets/fonts/." << std::endl;
    }
}

void TitleScreenApp::handleEvents() {
    // SFML3 pollEvent() returns std::optional<sf::Event>
    while (true) {
        auto evOpt = window->pollEvent();
        if (!evOpt.has_value()) break;
        auto ev = *evOpt;

        // Handle window closed (X) event (SFML3 variant API)
        if (ev.is<sf::Event::Closed>()) {
            window->close();
            return;
        }
        // Other events can be handled here if needed
    }

    // Use real-time input for simple checks (SFML3 changed event API)
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
        if (titleScreen->getCurrentState() == TitleScreen::State::Credits) {
            titleScreen->reset();
        }
    }
    
    if (!window->isOpen()) return;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) {
        window->close();
    }
}

int TitleScreenApp::run() {
    sf::Clock clock;
    
    while (window->isOpen()) {
        handleEvents();
        
        float deltaTime = clock.restart().asSeconds();
        
        sf::Vector2f mousePos(
            static_cast<float>(sf::Mouse::getPosition(*window).x),
            static_cast<float>(sf::Mouse::getPosition(*window).y)
        );
        
        bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
        
        titleScreen->update(deltaTime, mousePos, mousePressed);
        
        // Check title screen state
        if (titleScreen->shouldStartGame()) {
            std::cout << "Game started!" << std::endl;
            // TODO: Transition to main game loop
            break;
        }
        
        if (titleScreen->shouldExit()) {
            window->close();
        }
        
        // Render
        window->clear(sf::Color::Black);
        titleScreen->draw(*window);
        window->display();
    }
    
    return 0;
}
