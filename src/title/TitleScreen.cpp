#include "TitleScreen.h"
#include <cmath>
#include <random>
#include <iostream>

TitleScreen::TitleScreen(const std::string& backgroundPath)
    : currentState(State::FadingIn),
      fadeAlpha(0.f),
      maxFade(255.f),
      fadeSpeed(200.f),
      shouldStart(false),
      shouldExitGame(false),
      exitButtonEvading(false),
      evasionTimer(0.f) {
    
    // Load background: try requested path, then common alternatives
    bool loaded = backgroundTexture.loadFromFile(backgroundPath);
    if (!loaded) {
        // try same path with jpg
        std::string alt = backgroundPath;
        auto pos = alt.rfind('.');
        if (pos != std::string::npos) alt = alt.substr(0, pos) + ".jpg";
        loaded = backgroundTexture.loadFromFile(alt);
    }
    if (!loaded) {
        // try a commonly-named sample file
        loaded = backgroundTexture.loadFromFile("assets/textures/title_bg_sample.jpg");
    }
    if (!loaded) {
        // fallback: create a simple colored image
        sf::Image fallback;
        fallback.resize(sf::Vector2u(1920, 1080), sf::Color(30, 50, 80));
        backgroundTexture.loadFromImage(fallback);
    }
    
    backgroundSprite = new sf::Sprite(backgroundTexture);
    
    // Scale background to fit window
    auto textureSize = backgroundTexture.getSize();
    backgroundSprite->setScale(sf::Vector2f(1920.f / textureSize.x, 1080.f / textureSize.y));
    
    // Create buttons (order: START, RESET, CREDIT, LEAVE)
    // Compute vertical positions so spacing is equal
    float btnWidth = 320.f;
    float btnHeight = 80.f;
    const int count = 4;
    float spacing = 40.f; // will be overwritten to equal spacing
    float areaHeight = count * btnHeight + (count - 1) * spacing;
    // center vertical group around ~centerY (choose 520 for visual balance)
    float centerY = 520.f;
    // compute equal spacing so total group fits nicely
    float available = 520.f; // temporary placeholder
    // compute starting Y so buttons are centered around centerY
    float totalHeight = count * btnHeight + (count - 1) * spacing;
    float startY = centerY - totalHeight / 2.f;
    // Create buttons at computed positions
    startButton = std::make_unique<Button>(800.f, startY + 0 * (btnHeight + spacing), btnWidth, btnHeight, "START");
    resetButton = std::make_unique<Button>(800.f, startY + 1 * (btnHeight + spacing), btnWidth, btnHeight, "RESET");
    creditsButton = std::make_unique<Button>(800.f, startY + 2 * (btnHeight + spacing), btnWidth, btnHeight, "CREDIT");
    exitButton = std::make_unique<Button>(800.f, startY + 3 * (btnHeight + spacing), btnWidth, btnHeight, "LEAVE");
    
    exitButtonHomePos = exitButton->getPosition();
    exitButtonTargetPos = exitButton->getPosition();
    shouldResetGame = false;
    
    // Create credits screen
    creditsScreen = std::make_unique<CreditsScreen>();

    // Confirmation modal buttons (created without font; font set later)
    confirmYesButton = std::make_unique<Button>(1130.f, 500.f, 120.f, 60.f, "YES");
    confirmNoButton = std::make_unique<Button>(1310.f, 500.f, 120.f, 60.f, "NO");
    confirmText = nullptr;
}

void TitleScreen::setFont(const sf::Font& font) {
    startButton->setFont(font);
    creditsButton->setFont(font);
    exitButton->setFont(font);
    resetButton->setFont(font);
    confirmYesButton->setFont(font);
    confirmNoButton->setFont(font);
    creditsScreen->setFont(font);
    // setup confirm text for the reset confirmation (sf::Text requires a Font at construction in SFML3)
    if (confirmText) delete confirmText;
    confirmText = new sf::Text(font, "Are you sure you want to reset saved game?", 28);
    confirmText->setFillColor(sf::Color::White);
    creditsScreen->setFont(font);
}

void TitleScreen::update(float deltaTime, const sf::Vector2f& mousePos, bool mousePressed) {
    switch (currentState) {
        case State::FadingIn:
            fadeAlpha = std::min(maxFade, fadeAlpha + fadeSpeed * deltaTime);
            if (fadeAlpha >= maxFade) {
                currentState = State::Active;
            }
            break;
            
        case State::Active:
                    startButton->update(mousePos, deltaTime);
                    creditsButton->update(mousePos, deltaTime);
                    exitButton->update(mousePos, deltaTime);
            // If mouse is near the exit button, start evading
            {
                auto btnPos = exitButton->getPosition();
                auto btnBounds = exitButton->getBounds();
                sf::Vector2f btnCenter(
                    btnPos.x + btnBounds.size.x / 2.f,
                    btnPos.y + btnBounds.size.y / 2.f
                );
                float dx = btnCenter.x - mousePos.x;
                float dy = btnCenter.y - mousePos.y;
                float dist = std::sqrt(dx*dx + dy*dy);
                if (dist < 200.f) {
                    if (!exitButtonEvading) {
                        exitButtonEvading = true;
                        evasionTimer = 0.f;
                    }
                }
            }

            // handle reset button
            resetButton->update(mousePos, deltaTime);

            if (mousePressed) {
                if (startButton->isHovered()) {
                    // Start immediately; no confirmation needed
                    shouldStart = true;
                    currentState = State::Transitioning;
                } else if (resetButton->isHovered()) {
                    // Ask for confirmation before resetting
                    currentState = State::ConfirmReset;
                    if (confirmText) confirmText->setString("Are you sure you want to reset saved game?");
                } else if (creditsButton->isHovered()) {
                    currentState = State::Credits;
                    creditsScreen->show();
                } else if (exitButton->isHovered()) {
                    // Click LEAVE -> request exit immediately
                    shouldExitGame = true;
                }
            }

            updateExitButtonEvasion(mousePos, deltaTime);
            break;
            
        case State::Transitioning:
            fadeAlpha = std::max(0.f, fadeAlpha - fadeSpeed * deltaTime);
            break;
            
        case State::Credits:
            creditsScreen->update(deltaTime);
            if (creditsScreen->shouldReturnToTitle()) {
                currentState = State::Active;
                creditsScreen->hide();
            }
            break;

        case State::ConfirmStart:
            // Update confirm modal buttons
            confirmYesButton->update(mousePos, deltaTime);
            confirmNoButton->update(mousePos, deltaTime);
            if (mousePressed) {
                if (confirmYesButton->isHovered()) {
                    shouldStart = true;
                    currentState = State::Transitioning;
                } else if (confirmNoButton->isHovered()) {
                    currentState = State::Active;
                }
            }
            break;
        case State::ConfirmReset:
            confirmYesButton->update(mousePos, deltaTime);
            confirmNoButton->update(mousePos, deltaTime);
            if (mousePressed) {
                if (confirmYesButton->isHovered()) {
                    resetGame();
                    currentState = State::Active;
                } else if (confirmNoButton->isHovered()) {
                    currentState = State::Active;
                }
            }
            break;
    }
}

void TitleScreen::resetGame() {
    // Placeholder: reset saved game state. For now, print and set flag.
    std::cout << "Game reset!" << std::endl;
    shouldResetGame = true;
}

void TitleScreen::updateExitButtonEvasion(const sf::Vector2f& mousePos, float deltaTime) {
    auto buttonPos = exitButton->getPosition();
    auto buttonBounds = exitButton->getBounds();
    sf::Vector2f buttonCenter(
        buttonPos.x + buttonBounds.size.x / 2.f,
        buttonPos.y + buttonBounds.size.y / 2.f
    );

    // Direction from mouse to button center
    float dx = buttonCenter.x - mousePos.x;
    float dy = buttonCenter.y - mousePos.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance >= 200.f) {
        exitButtonEvading = false;
        evasionTimer = 0.f;
        exitButtonTargetPos = exitButtonHomePos;
    } else {
        evasionTimer += deltaTime;

        // Stop evading after 5 seconds, then return home
        if (evasionTimer > 5.f) {
            exitButtonEvading = false;
            exitButtonTargetPos = exitButtonHomePos;
        } else {
            exitButtonEvading = true;

            // Desired offset grows when closer
            float push = (200.f - distance) / 200.f * 220.f; // up to ~220px

            // Calculate target position using direction
            float angle = std::atan2(dy, dx);
            sf::Vector2f targetCenter(
                buttonCenter.x + std::cos(angle) * push,
                buttonCenter.y + std::sin(angle) * push
            );

            // Convert center to top-left
            exitButtonTargetPos = sf::Vector2f(
                std::max(50.f, std::min(targetCenter.x - buttonBounds.size.x / 2.f, 1920.f - buttonBounds.size.x - 50.f)),
                std::max(50.f, std::min(targetCenter.y - buttonBounds.size.y / 2.f, 1080.f - buttonBounds.size.y - 50.f))
            );
        }
    }

    // Smoothly move current position toward target (damped spring / lerp)
    auto cur = buttonPos;
    float stiffness = 10.f; // responsiveness
    float t = 1.f - std::exp(-stiffness * std::max(0.0f, deltaTime));
    float nx = cur.x + (exitButtonTargetPos.x - cur.x) * t;
    float ny = cur.y + (exitButtonTargetPos.y - cur.y) * t;
    exitButton->setPosition(nx, ny);
}

void TitleScreen::draw(sf::RenderWindow& window) {
    // Draw background
    if (backgroundSprite) window.draw(*backgroundSprite);
    
    // Draw dimming overlay based on fade state
    sf::RectangleShape dimOverlay(sf::Vector2f(window.getSize().x, window.getSize().y));
    float dimAmount = 1.f - (fadeAlpha / maxFade);
    dimOverlay.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(dimAmount * 150.f)));
    window.draw(dimOverlay);
    
    switch (currentState) {
        case State::FadingIn:
        case State::Active:
        case State::Transitioning:
            startButton->draw(window);
            resetButton->draw(window);
            creditsButton->draw(window);
            exitButton->draw(window);
            break;
            
        case State::Credits:
            // Keep title background visible, draw credits on top with dimming
            creditsScreen->draw(window);
            break;
        case State::ConfirmStart:
        case State::ConfirmReset:
            // dim and draw modal
            startButton->draw(window);
            resetButton->draw(window);
            creditsButton->draw(window);
            exitButton->draw(window);

            // Semi-opaque backdrop for modal
            {
                sf::RectangleShape modalBg(sf::Vector2f(620.f, 220.f));
                modalBg.setFillColor(sf::Color(0, 0, 0, 200));
                modalBg.setPosition(sf::Vector2f(1080.f, 360.f));
                window.draw(modalBg);

                // center confirm text
                if (confirmText) {
                    auto tb = confirmText->getLocalBounds();
                    confirmText->setPosition(sf::Vector2f(1080.f + 310.f - tb.size.x / 2.f, 390.f));
                    window.draw(*confirmText);
                }

                confirmYesButton->draw(window);
                confirmNoButton->draw(window);
            }
            break;
    }
}

void TitleScreen::reset() {
    currentState = State::FadingIn;
    fadeAlpha = 0.f;
    shouldStart = false;
    shouldExitGame = false;
    exitButtonEvading = false;
    evasionTimer = 0.f;
    exitButton->setPosition(exitButtonHomePos.x, exitButtonHomePos.y);
}
