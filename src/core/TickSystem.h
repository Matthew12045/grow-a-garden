#pragma once

#include <cstddef>

class TickSystem {
private:
    std::size_t currentTick_;
    float tickRate_;
    float timeAccumulator_;

public:
    TickSystem(float tickRate = 1.0f);

    void update(float deltaTime);
    std::size_t getTick() const;
    void fastForward(std::size_t ticks);
    void reset(std::size_t tick = 0);
    void setTickRate(float rate) { if (rate > 0.f) tickRate_ = rate; }
};
