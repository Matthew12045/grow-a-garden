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
    float triggerProbability_ = 0.4f;
    std::mt19937 rng_;

public:
    explicit RandomEventManager(float triggerProbability = 0.4f);
    ~RandomEventManager() = default;

    void registerEvent(std::unique_ptr<RandomEvent> event);
    void setTriggerProbability(float triggerProbability);
    void update(float dt, Garden& garden, Player& player);
};