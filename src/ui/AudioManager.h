#pragma once

#include "../world/WeatherSystem.h"

#include <SFML/Audio.hpp>
#include <algorithm>
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

public:
	// non-copyable: sf::Music is non-copyable
	AudioManager(const AudioManager&) = delete;
	AudioManager& operator=(const AudioManager&) = delete;

	// expose for tests
	static std::string weatherToTrackPath(WeatherType weatherType);
	AudioManager();

	void updateBGM(WeatherType currentWeather);
	void stop();
};
