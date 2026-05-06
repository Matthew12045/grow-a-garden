#include "TitleScreen.h"
#include <cmath>
#include <random>
#include <iostream>

TitleScreen::TitleScreen(const std::string& backgroundPath)
        : currentState(State::FadingIn),
            fadeAlpha(0.f),
            maxFade(255.f),
            fadeSpeed(200.f),
            introTimer(0.f),
            loadingTimer(0.f),
            loadingDuration(kLoadingDuration),
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
    
    // Load intro image (logo)
    bool logoLoaded = introTexture.loadFromFile("assets/textures/LOGO.png");
    if (!logoLoaded) {
        std::cerr << "Warning: Could not load LOGO.png, creating fallback logo" << std::endl;
        // Create a simple fallback logo texture
        sf::Image fallbackLogo;
        fallbackLogo.resize(sf::Vector2u(400, 200), sf::Color(100, 150, 200));
        // Add some text-like pattern
        for (unsigned int x = 50; x < 350; x += 10) {
            for (unsigned int y = 50; y < 150; y += 10) {
                fallbackLogo.setPixel(sf::Vector2u(x, y), sf::Color::White);
            }
        }
        logoLoaded = introTexture.loadFromImage(fallbackLogo);
    }
    introSprite = new sf::Sprite(introTexture);
    // Center and scale the logo
    auto logoSize = introTexture.getSize();
    if (logoSize.x > 0 && logoSize.y > 0) {
        float scale = std::min(1920.f / logoSize.x, 1080.f / logoSize.y) * 0.8f; // 80% of screen
        introSprite->setScale(sf::Vector2f(scale, scale));
        introSprite->setPosition(
            sf::Vector2f((1920.f - logoSize.x * scale) / 2.f,
                         (1080.f - logoSize.y * scale) / 2.f)
        );
    }
    
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
            // Fade in to reveal the logo
            fadeAlpha = std::min(maxFade, fadeAlpha + fadeSpeed * deltaTime);
            introTimer += deltaTime;
            if (fadeAlpha >= maxFade && introTimer >= kIntroFadeDuration) {
                introTimer = 0.f;
                currentState = State::IntroHoldLogo;
            }
            break;

        case State::IntroHoldLogo:
            // Hold the logo for a moment
            introTimer += deltaTime;
            if (introTimer >= kIntroHoldDuration) {
                introTimer = 0.f;
                currentState = State::IntroDimLogo;
            }
            break;

        case State::IntroDimLogo: {
            // Dim the logo and fade to black, then go to loading screen
            introTimer += deltaTime;
            float dimProgress = introTimer / kIntroDimDuration;
            fadeAlpha = maxFade * (1.f - dimProgress); // Fade from white to black
            if (introTimer >= kIntroDimDuration) {
                fadeAlpha = 0.f;
                introTimer = 0.f;
                loadingTimer = 0.f;
                currentState = State::IntroLoading;
            }
            break;
        }

        case State::IntroLoading:
            loadingTimer += deltaTime;
            if (loadingTimer >= loadingDuration) {
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
    sf::Vector2u sz = window.getSize();

    // Handle intro states with logo and transitions
    if (currentState == State::FadingIn || currentState == State::IntroHoldLogo || 
        currentState == State::IntroDimLogo) {
        // Draw black background
        sf::RectangleShape black(sf::Vector2f(static_cast<float>(sz.x), static_cast<float>(sz.y)));
        black.setFillColor(sf::Color::Black);
        window.draw(black);

        // Draw logo with fade
        if (introSprite) {
            // Calculate alpha for logo (inverted from fadeAlpha)
            // When fadeAlpha goes from 0 to 255, logo goes from 0 to 255
            // When fadeAlpha goes from 255 to 0 (dimming), logo goes from 255 to 0
            float alpha = std::min(255.f, fadeAlpha);
            introSprite->setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(alpha)));
            window.draw(*introSprite);
        }
        return;
    }

    if (currentState == State::IntroLoading) {
        // Draw black background
        sf::RectangleShape black(sf::Vector2f(static_cast<float>(sz.x), static_cast<float>(sz.y)));
        black.setFillColor(sf::Color::Black);
        window.draw(black);

        // Loading text
        if (confirmText) {
            const sf::Font& ft = confirmText->getFont();
            unsigned int size = 28;
            sf::Text loadingText(ft, "Loading...", size);
            loadingText.setFillColor(sf::Color::White);
            auto lb = loadingText.getLocalBounds();
            float lx = static_cast<float>(sz.x) / 2.f - lb.size.x / 2.f;
            float ly = static_cast<float>(sz.y) / 2.f - 48.f;
            loadingText.setPosition(sf::Vector2f(lx, ly));
            window.draw(loadingText);
        }

        // Progress bar (white)
        float progress = std::min(1.f, loadingTimer / loadingDuration);
        float barW = 600.f;
        float barH = 24.f;
        float bx = static_cast<float>(sz.x) / 2.f - barW / 2.f;
        float by = static_cast<float>(sz.y) / 2.f - barH / 2.f;

        // Filled portion
        sf::RectangleShape fill(sf::Vector2f(barW * progress, barH));
        fill.setPosition(sf::Vector2f(bx, by));
        fill.setFillColor(sf::Color::White);
        window.draw(fill);

        // Outline
        sf::RectangleShape outline(sf::Vector2f(barW, barH));
        outline.setPosition(sf::Vector2f(bx, by));
        outline.setFillColor(sf::Color::Transparent);
        outline.setOutlineColor(sf::Color::White);
        outline.setOutlineThickness(2.f);
        window.draw(outline);

        return;
    }

    // Draw normal title screen with buttons (Active, Transitioning, etc)
    if (backgroundSprite) window.draw(*backgroundSprite);
    
    switch (currentState) {
        case State::Active:
        case State::Transitioning:
            startButton->draw(window);
            resetButton->draw(window);
            creditsButton->draw(window);
            exitButton->draw(window);
            break;
            
        case State::Credits:
            // Keep title background visible, draw credits on top
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
        
        // Intro states handled above, shouldn't reach here
        case State::FadingIn:
        case State::IntroHoldLogo:
        case State::IntroDimLogo:
        case State::IntroLoading:
        default:
            break;
    }
}

void TitleScreen::reset() {
    currentState = State::FadingIn;
    fadeAlpha = 0.f;
    introTimer = 0.f;
    loadingTimer = 0.f;
    shouldStart = false;
    shouldExitGame = false;
    exitButtonEvading = false;
    evasionTimer = 0.f;
    exitButton->setPosition(exitButtonHomePos.x, exitButtonHomePos.y);
}
