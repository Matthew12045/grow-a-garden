#pragma once

#include "TickSystem.h"
#include "../world/Garden.h"
#include "../world/WeatherSystem.h"
#include "Player.h"
#include "../systems/RandomEventManager.h"

#include <cstdint>
#include <memory>
#include <vector>

struct BasketEntry;

class Game {
private:
    TickSystem tickSystem_;
    Garden garden_;
    WeatherSystem weatherSystem_;
    Player player_;
    RandomEventManager randEventMgr_;
    std::int64_t lastSaveTimestamp_;
    bool loadedSave_;
    bool initialized_;
    std::vector<BasketEntry>* harvestBasket_ = nullptr;

public:
    Game();

    void update(float deltaTime);
    void autoSave();
    void saveGame();
    void loadGame();
    void processOfflineProgress();
    bool hasLoadedSave() const { return loadedSave_; }
    bool isInitialized() const { return initialized_; }
    void setInitialized(bool initialized) { initialized_ = initialized; }
    bool hasHarvestBasket() const { return harvestBasket_ != nullptr; }

    // Non-owning save hook. The bound basket must outlive Game or be cleared
    // with unbindHarvestBasket() before destruction.
    void bindHarvestBasket(std::vector<BasketEntry>& harvestBasket);
    void unbindHarvestBasket();

    // Getters for testing/access
    TickSystem& getTickSystem() { return tickSystem_; }
    Garden& getGarden() { return garden_; }
    WeatherSystem& getWeatherSystem() { return weatherSystem_; }
    Player& getPlayer() { return player_; }
};
