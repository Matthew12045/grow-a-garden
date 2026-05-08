#pragma once

// ── Concrete Plant ─────────────────────────────────────────────────────────
// Convenience subclass used by focused unit tests that need a named concrete
// plant fixture without depending on catalogue data.
#include "../../src/entities/Plant.h"

class ConcretePlant : public Plant {
public:
    ConcretePlant(int          id,
                  std::string  name,
                  int          currentStage,
                  int          maxStages,
                  std::size_t  ticksPerStage,
                  std::size_t  ticksElapsed,
                  double       sellPrice,
                  bool         regrows      = false,
                  int          regrowStage  = 0)
        : Plant(id, std::move(name), currentStage, maxStages,
                ticksPerStage, ticksElapsed, sellPrice, regrows, regrowStage) {}
};

// ── Concrete Tool ──────────────────────────────────────────────────────────
#include "../../src/items/Tool.h"
#include "../../src/core/Player.h"
#include "../../src/world/Cell.h"

class ConcreteTool : public Tool {
public:
    mutable int useCalls = 0;

    ConcreteTool(std::size_t id,
                 std::string  name,
                 std::string  desc,
                 double       price,
                 int          durability)
        : Tool(id, std::move(name), std::move(desc), price, durability) {}

    void use(Cell& /*cell*/, Player& /*player*/) override { ++useCalls; }
};
