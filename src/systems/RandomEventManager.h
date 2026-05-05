#pragma once

#include <vector>
#include <memory>
#include "RandomEvent.h"

class Garden;
class Player;

class RandomEventManager {
private:
    std::vector<std::unique_ptr<RandomEvent>> events_;
    float triggerProbability_;
    int cooldownTicks_;
    int lastTriggeredTick_;

public:
    RandomEventManager(float triggerProb, int cooldownTicks);

    void addEvent(std::unique_ptr<RandomEvent> event);
    void checkAndTrigger(int tick, Garden& garden, Player& player);
};