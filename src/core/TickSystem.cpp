#include "TickSystem.h"

TickSystem::TickSystem(float tickRate)
    : currentTick_(0), tickRate_(tickRate), timeAccumulator_(0.0f) {}

void TickSystem::update(float deltaTime) {
    if (tickRate_ <= 0.0f) return;

    timeAccumulator_ += deltaTime;

    // Process as many ticks as fit into the accumulated time
    while (timeAccumulator_ >= tickRate_) {
        currentTick_++;
        timeAccumulator_ -= tickRate_;
    }
}

std::size_t TickSystem::getTick() const {
    return currentTick_;
}

void TickSystem::fastForward(std::size_t ticks) {
    currentTick_ += ticks;
}

void TickSystem::reset(std::size_t tick) {
    currentTick_ = tick;
    timeAccumulator_ = 0.0f;
}
