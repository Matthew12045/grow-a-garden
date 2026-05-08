#pragma once

#include "RandomEvent.h"
#include <random>

class RaccoonEvent : public RandomEvent {
private:
    int stolenAmount_;
    std::mt19937 rng_;

public:
    explicit RaccoonEvent(int stolenAmount = 10);
    ~RaccoonEvent() override = default;

    void trigger(Garden& garden, Player& player) override;
};