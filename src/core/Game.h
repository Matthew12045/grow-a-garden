#pragma once

#include "TickSystem.h"
#include "../world/Garden.h"
#include "../world/WeatherSystem.h"
#include "Player.h"
// #include "../systems/Shop.h"
// #include "../ui/UIManager.h"
// #include "../ui/AudioManager.h"
#include "../systems/RandomEventManager.h"

#include <memory>
#include <vector>

struct BasketEntry;

class Game {
private:
    TickSystem tickSystem_;
    Garden garden_;
    WeatherSystem weatherSystem_;
    Player player_;
    // Shop shop_;
    // UIManager uiManager_;
    // AudioManager audioManager_;
    RandomEventManager randEventMgr_;
    long lastSaveTimestamp_;
    bool loadedSave_;
    // Non-owning save hook for the UI harvest basket. The bound basket must
    // outlive Game or be cleared with unbindHarvestBasket() before destruction.
    std::vector<BasketEntry>* harvestBasket_;

public:
    Game();

    void update(float deltaTime);
    void autoSave();
    void saveGame();
    void loadGame();
    void processOfflineProgress();
    bool hasLoadedSave() const { return loadedSave_; }
    bool hasHarvestBasket() const { return harvestBasket_ != nullptr; }
    void bindHarvestBasket(std::vector<BasketEntry>& harvestBasket);
    void unbindHarvestBasket();

    // Getters for testing/access
    TickSystem& getTickSystem() { return tickSystem_; }
    Garden& getGarden() { return garden_; }
    WeatherSystem& getWeatherSystem() { return weatherSystem_; }
    Player& getPlayer() { return player_; }
};
