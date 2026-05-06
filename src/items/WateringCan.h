#pragma once

#include "../items/Tool.h"

// WateringCan is the first concrete Tool. On use() it immediately advances
// the target plant by a fixed number of ticks, simulating accelerated growth.
//
// NOTE on Item's protected interface:
//   Item::getName(), getDescr(), and id() are protected. Systems like
//   Inventory that hold Item* need those methods to be public. WateringCan
//   lifts them via `using` declarations. Seed and HarvestedItem will need
//   the same treatment once the full build is wired together.
class WateringCan : public Tool {
public:
    static constexpr int GROWTH_BOOST_TICKS = 5;  // ticks applied per use

    WateringCan();

    // ── Lift Item's protected accessors to public ────────────────────────
    using Tool::getPrice;
    using Tool::getDurability;

    // These live on Item but are protected there; expose them here so
    // Inventory (and any other system that holds an Item*) can reach them.
    using Item::getName;
    using Item::getDescr;
    using Item::id;

    // Tool::use — grows the plant in `cell` by GROWTH_BOOST_TICKS ticks.
    // Because Plant::grow() is protected, we need a Carrot* (or any concrete
    // subclass that publicly re-exports grow()) to call it. The cast is
    // intentional here: the game will only give tools a cell that holds the
    // crop type the tool was designed for.
    void use(Cell& cell, Player& player) override;
};
