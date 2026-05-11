#include "WateringCan.h"
#include "../world/Cell.h"
#include "../core/Player.h"

WateringCan::WateringCan(double basePrice)
    : Tool(/*id*/          1,
           /*name*/        "Watering Can",
           /*description*/ "Waters crops — instantly advances growth by 5 ticks.",
           /*basePrice*/   basePrice,
           /*durability*/  50) {}

std::unique_ptr<Item> WateringCan::clone() const
{
    return std::make_unique<WateringCan>(*this);
}

void WateringCan::use(Cell& cell, Player& /*player*/)
{
    if (isBroken()) {
        return;
    }

    Plant* plant = cell.getPlant();
    if (plant == nullptr) {
        return;  // nothing to water
    }
    if (plant->isFullyGrown()) {
        return;
    }

    plant->grow(static_cast<std::size_t>(GROWTH_BOOST_TICKS));
    consumeDurability();
}
