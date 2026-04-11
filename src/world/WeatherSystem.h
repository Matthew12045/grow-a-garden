#pragma once

#include <random>

enum class WeatherType {
	SUMMER,
	RAIN,
	FROST,
	THUNDER_STORM,
	METEOR_SHOWER
};

class Garden;
class Plant;

class WeatherSystem {
private:
	WeatherType currentWeather_;
	int ticksUntilChange_;
	std::mt19937 rng_;

	WeatherType rollNextWeather();
	int rollTicksUntilChange();

public:
	WeatherSystem();

	void update();
	WeatherType getCurrentWeather() const;
	void applyEffectsToGrid(Garden& garden);
	void tryTriggerMutation(Plant& plant);
};
