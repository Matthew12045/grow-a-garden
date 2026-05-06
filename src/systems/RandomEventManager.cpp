#include "RandomEventManager.h"
#include <cstdlib>

RandomEventManager::RandomEventManager(float triggerProb, int cooldownTicks)
    : triggerProbability_(triggerProb), cooldownTicks_(cooldownTicks), lastTriggeredTick_(-cooldownTicks) {}

void RandomEventManager::addEvent(std::unique_ptr<RandomEvent> event) {
    if (event) {
        events_.push_back(std::move(event));
    }
}

void RandomEventManager::checkAndTrigger(int tick, Garden& garden, Player& player) {
    if (events_.empty()) return;

    if (tick - lastTriggeredTick_ >= cooldownTicks_) {
        // Roll dice
        float roll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        if (roll <= triggerProbability_) {
            // Pick random event
            int index = rand() % events_.size();
            events_[index]->trigger(garden, player);
            lastTriggeredTick_ = tick;
        }
    }
}