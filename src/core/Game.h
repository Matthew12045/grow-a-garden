#pragma once

#include "TickSystem.h"
#include "../world/Garden.h"
#include "../world/WeatherSystem.h"
#include "Player.h"
// #include "../systems/Shop.h"
// #include "../ui/UIManager.h"
// #include "../ui/AudioManager.h"
// #include "../systems/RandomEventManager.h"

#include <memory>

class Game {
private:
    TickSystem tickSystem_;
    Garden garden_;
    WeatherSystem weatherSystem_;
    Player player_;
    // Shop shop_;
    // UIManager uiManager_;
    // AudioManager audioManager_;
    // RandomEventManager randEventMgr_;
    long lastSaveTimestamp_;

public:
    Game();

    void update(float deltaTime);
    void autoSave();
    void saveGame();
    void loadGame();
    void processOfflineProgress();

    // Getters for testing/access
    TickSystem& getTickSystem() { return tickSystem_; }
    Garden& getGarden() { return garden_; }
    WeatherSystem& getWeatherSystem() { return weatherSystem_; }
    Player& getPlayer() { return player_; }
};