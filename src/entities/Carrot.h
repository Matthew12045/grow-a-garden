#pragma once

#include "Plant.h"

// Carrot is the first concrete Plant. It publicly exposes all of Plant's
// protected interface so the rest of the game (Garden, WeatherSystem, Cell)
// can interact with it via Plant* while subclasses add crop-specific flavour.
class Carrot : public Plant {
public:
    // Default carrot: 5 growth stages, 10 ticks each, sells for 30 sheckles.
    // Does not regrow after harvest (set regrowsAfterHarvest = false).
    Carrot();

    // Named constructor for tests that need non-default stats.
    Carrot(int          id,
           int          currentStage,
           int          maxStages,
           std::size_t  ticksPerStage,
           std::size_t  ticksElapsed,
           double       sellPrice,
           bool         regrows     = false,
           int          regrowStage = 0);

    // ── Expose Plant's protected interface publicly ──────────────────────
    // Plant deliberately keeps these protected so only subclasses publish
    // them. Carrot makes them visible to the game systems that need them.
    using Plant::grow;
    using Plant::harvest;
    using Plant::isFullyGrown;
    using Plant::getTimeToGrowth;
    using Plant::applyWeatherEffect;
    using Plant::addMutation;
    using Plant::getMutations;
};
