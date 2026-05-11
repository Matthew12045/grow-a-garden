#include "FertilizerTool.h"

#include "../core/Player.h"
#include "../world/Cell.h"

FertilizerTool::FertilizerTool()
    : Tool(/*id*/          2,
           /*name*/        "Fertilizer",
           /*description*/ "Fertilizes crops - instantly advances growth by 20 ticks.",
           /*basePrice*/   50.0,
           /*durability*/  25) {}

std::unique_ptr<Item> FertilizerTool::clone() const
{
    return std::make_unique<FertilizerTool>(*this);
}

void FertilizerTool::use(Cell& cell, Player& /*player*/)
{
    if (isBroken()) {
        return;
    }

    Plant* plant = cell.getPlant();
    if (plant == nullptr) {
        return;
    }
    if (plant->isFullyGrown()) {
        return;
    }

    plant->grow(static_cast<std::size_t>(GROWTH_BOOST_TICKS));
    consumeDurability();
}
