#include "Plant.h"

#include "HarvestedItem.h"


Plant::Plant(int id,
    const std::string name,
    int currentStage,
    int maxStages,
    std::size_t ticksPerStage,
    std::size_t ticksElapsed,
    double sellPrice,
    bool regrowsAfterHarvest,
    int regrowStage)
    : id_(id),
    name_(std::move(name)),
    currentStage_(currentStage),
    maxStages_(maxStages),
    baseTicksPerStage_(ticksPerStage),
    currentTicksPerStage_(ticksPerStage),
    ticksElapsed_(ticksElapsed),
    sellPrice_(sellPrice),
    mutations_(),
    regrowsAfterHarvest_(regrowsAfterHarvest),
    consumedOnHarvest_(true),
    regrowStage_(regrowStage) {}

void Plant::grow(std::size_t ticks)
{
    if (ticks == 0 || currentTicksPerStage_ == 0 || maxStages_ <= 0) {
        return;
    }

    const std::size_t maxTicks = static_cast<std::size_t>(maxStages_) * currentTicksPerStage_;
    if (ticksElapsed_ >= maxTicks) {
        ticksElapsed_ = maxTicks;
        currentStage_ = maxStages_;
        return;
    }

    const std::size_t remainingTicks = maxTicks - ticksElapsed_;
    const std::size_t ticksToApply = (ticks < remainingTicks) ? ticks : remainingTicks;
    ticksElapsed_ += ticksToApply;

    currentStage_ = static_cast<int>(ticksElapsed_ / currentTicksPerStage_);
    if (currentStage_ > maxStages_) {
        currentStage_ = maxStages_;
    }
}

HarvestedItem Plant::harvest()
{
    if (!isFullyGrown()) return HarvestedItem{};

    // copy mutations from plant to harvested item
    std::vector<MutationType> harvestedMutations;
    for (const auto& m : mutations_) {
        harvestedMutations.push_back(m.getType());
    }

    HarvestedItem item(calcPrice(), harvestedMutations);

    if (regrowsAfterHarvest_) {
        currentStage_ = regrowStage_;
        ticksElapsed_ = regrowStage_ * currentTicksPerStage_;
        mutations_.clear();
        consumedOnHarvest_ = false;
    } else {
        consumedOnHarvest_ = true;
    }

    return item;
}

bool Plant::isFullyGrown() const
{
    return (currentStage_ == maxStages_) ? true : false;
}

std::size_t Plant::getTimeToGrowth() const
{
    if (currentTicksPerStage_ == 0 || maxStages_ <= 0) {
        return 0;
    }

    const std::size_t maxTicks =
        static_cast<std::size_t>(maxStages_) * currentTicksPerStage_;

    if (ticksElapsed_ >= maxTicks || currentStage_ >= maxStages_) {
        return 0;
    }

    return maxTicks - ticksElapsed_;
}

void Plant::applyWeatherEffect(WeatherType weatherType)
{
    float multiplier = 1.0f;
    switch (weatherType) {
        case WeatherType::SUMMER:        multiplier = 1.00f; break;
        case WeatherType::RAIN:          multiplier = 1.50f; break;
        case WeatherType::FROST:         multiplier = 1.50f; break;
        case WeatherType::THUNDER_STORM: multiplier = 1.50f; break;
        case WeatherType::METEOR_SHOWER: multiplier = 1.75f; break;
        default: break;
    }


    const auto newTicks = static_cast<std::size_t>(
        std::round(static_cast<float>(baseTicksPerStage_) / multiplier)
    );
    currentTicksPerStage_ = (newTicks > 0) ? newTicks : 1;  // guard against 0
}

void Plant::addMutation(Mutation newMutation)
{
    // for(const auto& m : mutations_)
    // {
    //     if (newMutation.getType() == m.getType()) return;
    // }
    // mutations_.push_back(newMutation);

    const auto it = std::find_if(mutations_.begin(), mutations_.end(),
            [&](const Mutation& m) {
                return m.getType() == newMutation.getType();
            });

    if (it != mutations_.end()) {
        return;
    }

    mutations_.push_back(newMutation);
}


std::vector<MutationType> Plant::getMutations() const
{
    // mutation type
    std::vector<MutationType> mt;
    mt.reserve(mutations_.size());
    for (const auto& m : mutations_)
    {
        mt.push_back(m.getType());
    }
    return mt;
}

double Plant::calcPrice()
{
    // If the plant has no mutations, it just sells for the base price
    if (mutations_.empty()) {
        return sellPrice_;
    }

    double finalPrice = 0.0;

    // Multiply the base price by each multiplier, then add them all up
    for (const auto& m : mutations_)
    {
        finalPrice += (sellPrice_ * m.getMultiplier());
    }

    return finalPrice;
}

const int Plant::getStage() const
{
    return currentStage_;
}