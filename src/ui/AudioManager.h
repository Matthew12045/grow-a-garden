#pragma once

#include "../world/WeatherSystem.h"

#include <SFML/Audio.hpp>
#include <algorithm>
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

class AudioManager {
private:
	WeatherType currentWeather_;
	bool isPlaying_;
	sf::Music currentTrack_;
	std::string currentTrackPath_;
	std::unordered_map<std::string, sf::SoundBuffer> soundBuffers_;
	std::vector<sf::Sound> activeSounds_;

	static std::vector<std::string> buildTrackCandidates(WeatherType weatherType);

public:
	// non-copyable: sf::Music is non-copyable
	AudioManager(const AudioManager&) = delete;
	AudioManager& operator=(const AudioManager&) = delete;

	// expose for tests
	static std::string weatherToTrackPath(WeatherType weatherType);
	AudioManager();

	void updateBGM(WeatherType currentWeather);
	bool playSound(const std::string& relativePath);
	void stop();
};
