#pragma once

class Garden;
class Player;

class RandomEvent {
protected:
    RandomEvent() = default;

public:
    virtual ~RandomEvent() = default;

    virtual void trigger(Garden& garden, Player& player) = 0;
};