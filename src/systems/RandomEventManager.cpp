#include "RandomEventManager.h"
#include "RandomEvent.h"
#include "../world/Garden.h"
#include "../core/Player.h"

// Constants for easy tuning
static constexpr float TRIGGER_INTERVAL = 60.0f;  // fire every 60 seconds of real time

RandomEventManager::RandomEventManager(float triggerProbability)
    : triggerProbability_(triggerProbability),
      rng_(std::random_device{}()) {}

void RandomEventManager::registerEvent(std::unique_ptr<RandomEvent> event) {
    if (event) {
        events_.push_back(std::move(event));
    }
}

void RandomEventManager::setTriggerProbability(float triggerProbability) {
    triggerProbability_ = triggerProbability;
}

void RandomEventManager::update(float dt, Garden& garden, Player& player) {
    if (events_.empty()) return;

    elapsedTime_ += dt;

    while (elapsedTime_ >= TRIGGER_INTERVAL) {
        elapsedTime_ -= TRIGGER_INTERVAL;

        std::bernoulli_distribution shouldTrigger(triggerProbability_);
        if (!shouldTrigger(rng_)) {
            continue;
        }

        // Pick a random event from the registered list
        std::uniform_int_distribution<std::size_t> dist(0, events_.size() - 1);
        std::size_t index = dist(rng_);
        events_[index]->trigger(garden, player);
    }
}
