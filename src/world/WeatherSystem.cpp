#include "WeatherSystem.h"

#include "Garden.h"
#include "Cell.h"
#include "../entities/Mutation.h"
#include "../entities/Plant.h"

namespace {
Mutation buildMutationForWeather(WeatherType weather)
{
	switch (weather) {
	case WeatherType::RAIN:
		return Mutation(MutationType::WET, 2.0f, WeatherType::RAIN);
	case WeatherType::FROST:
		return Mutation(MutationType::FROZEN, 5.0f, WeatherType::FROST);
	case WeatherType::THUNDER_STORM:
		return Mutation(MutationType::SHOCKED, 100.0f, WeatherType::THUNDER_STORM);
	case WeatherType::METEOR_SHOWER:
		return Mutation(MutationType::CELESTIAL, 150.0f, WeatherType::METEOR_SHOWER);
	default:
		return Mutation(MutationType::WET, 1.0f, WeatherType::SUMMER);
	}
}

double mutationChanceForWeather(WeatherType weather)
{
	switch (weather) {
	case WeatherType::RAIN:
		return 0.08;
	case WeatherType::FROST:
		return 0.05;
	case WeatherType::THUNDER_STORM:
		return 0.02;
	case WeatherType::METEOR_SHOWER:
		return 0.01;
	default:
		return 0.0;
	}
}
} // namespace

WeatherSystem::WeatherSystem()
	: currentWeather_(WeatherType::SUMMER),
	  ticksUntilChange_(20),
	  rng_(std::random_device{}()) {}

void WeatherSystem::update()
{
	ticksUntilChange_--;
	if (ticksUntilChange_ > 0) {
		return;
	}

	currentWeather_ = rollNextWeather();
	ticksUntilChange_ = rollTicksUntilChange();
}

WeatherType WeatherSystem::getCurrentWeather() const
{
	return currentWeather_;
}

void WeatherSystem::applyEffectsToGrid(Garden& garden)
{
	for (int y = 0; y < garden.getHeight(); ++y) {
		for (int x = 0; x < garden.getWidth(); ++x) {
			Cell& cell = garden.getCell(x, y);
			Plant* plant = cell.getPlant();
			if (plant == nullptr) {
				continue;
			}

			plant->applyWeatherEffect(currentWeather_);
			tryTriggerMutation(*plant);
		}
	}
}

void WeatherSystem::tryTriggerMutation(Plant& plant)
{
	const double chance = mutationChanceForWeather(currentWeather_);
	if (chance <= 0.0) {
		return;
	}

	std::bernoulli_distribution roll(chance);
	if (!roll(rng_)) {
		return;
	}

	plant.addMutation(buildMutationForWeather(currentWeather_));
}

WeatherType WeatherSystem::rollNextWeather()
{
	std::uniform_int_distribution<int> dist(0, 4);
	return static_cast<WeatherType>(dist(rng_));
}

int WeatherSystem::rollTicksUntilChange()
{
	std::uniform_int_distribution<int> dist(15, 35);
	return dist(rng_);
}
