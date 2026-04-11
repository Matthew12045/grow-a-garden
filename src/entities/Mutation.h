#pragma once

#include "../world/WeatherSystem.h"

class Plant;

enum class MutationType 
{
	WET,
	SHOCKED,
	FROZEN,
	CELESTIAL
};

class Mutation 
{
    private:
        MutationType type_;
        float multiplier_;
        WeatherType triggerWeather_;
    public:
        Mutation(MutationType type, float multiplier, WeatherType trigger);
        MutationType getType() const;
        float getMultiplier() const;
        WeatherType getTrigger() const;
        void apply(Plant& plant);
};