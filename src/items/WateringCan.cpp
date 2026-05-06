#include "WateringCan.h"
#include "../entities/Carrot.h"
#include "../world/Cell.h"
#include "../core/Player.h"

WateringCan::WateringCan()
    : Tool(/*id*/          1,
           /*name*/        "Watering Can",
           /*description*/ "Waters crops — instantly advances growth by 5 ticks.",
           /*basePrice*/   25.0,
           /*durability*/  50) {}

void WateringCan::use(Cell& cell, Player& /*player*/)
{
    Plant* plant = cell.getPlant();
    if (plant == nullptr) {
        return;  // nothing to water
    }

    // Attempt a safe downcast to Carrot (the only concrete Plant so far).
    // When more crop types are added each should publicly expose grow(),
    // or Plant itself should gain a public virtual tick() interface.
    if (auto* carrot = dynamic_cast<Carrot*>(plant)) {
        carrot->grow(static_cast<std::size_t>(GROWTH_BOOST_TICKS));
    }
}
