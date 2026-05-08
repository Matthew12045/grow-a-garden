#pragma once

#include <vector>
#include <memory>
#include <random>
#include "RandomEvent.h"

class Garden;
class Player;

class RandomEventManager {
private:
    std::vector<std::unique_ptr<RandomEvent>> events_;
    float elapsedTime_ = 0.0f;
    std::mt19937 rng_;

public:
    RandomEventManager();
    ~RandomEventManager() = default;

    void registerEvent(std::unique_ptr<RandomEvent> event);
    void update(float dt, Garden& garden, Player& player);
};