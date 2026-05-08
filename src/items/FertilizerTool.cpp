#include "FertilizerTool.h"

#include "../core/Player.h"
#include "../world/Cell.h"

FertilizerTool::FertilizerTool()
    : Tool(/*id*/          2,
           /*name*/        "Fertilizer",
           /*description*/ "Fertilizes crops - instantly advances growth by 20 ticks.",
           /*basePrice*/   50.0,
           /*durability*/  25) {}

void FertilizerTool::use(Cell& cell, Player& /*player*/)
{
    Plant* plant = cell.getPlant();
    if (plant == nullptr) {
        return;
    }

    plant->grow(static_cast<std::size_t>(GROWTH_BOOST_TICKS));
}
