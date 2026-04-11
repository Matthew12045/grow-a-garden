#include "Mutation.h"

#include "Plant.h"

Mutation::Mutation(MutationType type, float multiplier, WeatherType trigger) 
    : type_(type),
      multiplier_(multiplier),
      triggerWeather_(trigger) {}

MutationType Mutation::getType() const
{
    return type_;
}

float Mutation::getMultiplier() const
{
    return multiplier_;
}

WeatherType Mutation::getTrigger() const
{
    return triggerWeather_;
}

void Mutation::apply(Plant& plant)
{
    // Mutation modifies the plant — e.g. registers itself
    plant.addMutation(*this);
}
