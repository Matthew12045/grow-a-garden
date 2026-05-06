#pragma once

#include "RandomEvent.h"

class RaccoonEvent : public RandomEvent {
private:
    int stolenAmount_;

public:
    explicit RaccoonEvent(int stolenAmount = 10);
    ~RaccoonEvent() override = default;

    void trigger(Garden& garden, Player& player) override;
};