#pragma once

#include "../world/WeatherSystem.h"

#include <SFML/Audio.hpp>
#include <array>
#include <string>
#include <vector>

class AudioManager {
private:
	WeatherType currentWeather_;
	bool isPlaying_;
	sf::Music currentTrack_;
	std::string currentTrackPath_;

	static std::vector<std::string> buildTrackCandidates(WeatherType weatherType);
	static std::string weatherToTrackPath(WeatherType weatherType);

public:
	AudioManager();

	void updateBGM(WeatherType currentWeather);
	void stop();
};
