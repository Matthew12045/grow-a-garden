#include "PlantFactory.h"
#include <algorithm>
#include <stdexcept>

std::unique_ptr<Plant> makePlant(const ShopItemDef& def) {
    if (def.type != ShopItemType::SEED) {
        throw std::runtime_error("makePlant requires seed catalogue entry: " + def.name);
    }

    const int maxStages = (def.maxStages > 0) ? def.maxStages : 1;
    const std::size_t ticksPerStage = (def.growTicks > 0 && maxStages > 0)
        ? std::max<std::size_t>(1u, static_cast<std::size_t>(def.growTicks / maxStages))
        : 1u;

    return std::make_unique<Plant>(
        0,
        def.cropName,
        0,
        maxStages,
        ticksPerStage,
        0,
        def.sellPrice,
        def.regrowsAfterHarvest,
        def.regrowStage
    );
}
