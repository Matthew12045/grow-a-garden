#include "AudioManager.h"

#include <filesystem>
#include <iostream>

AudioManager::AudioManager()
	: currentWeather_(WeatherType::SUMMER),
	  isPlaying_(false) {
}

std::string AudioManager::weatherToTrackPath(WeatherType weatherType) {
	switch (weatherType) {
		case WeatherType::SUMMER:
			return "assets/audio/bgm/06. Main Music 00.flac";
		case WeatherType::RAIN:
			return "assets/audio/bgm/09. Main Music 03.flac";
		case WeatherType::FROST:
			return "assets/audio/bgm/12. Main Music 06.flac";
		case WeatherType::THUNDER_STORM:
			return "assets/audio/bgm/16. Main Music 10.flac";
		case WeatherType::METEOR_SHOWER:
			return "assets/audio/bgm/19. Main Music 13.flac";
	}

	return "assets/audio/bgm/06. Main Music 00.flac";
}

std::vector<std::string> AudioManager::buildTrackCandidates(WeatherType weatherType) {
	const std::string preferred = weatherToTrackPath(weatherType);
	return {
		preferred,
		"assets/audio/bgm/06. Main Music 00.flac",
		"assets/audio/bgm/07. Main Music 01.flac",
		"assets/audio/bgm/08. Main Music 02.flac",
		"assets/audio/bgm/09. Main Music 03.flac",
		"assets/audio/bgm/10. Main Music 04.flac",
		"assets/audio/bgm/11. Main Music 05.flac",
		"assets/audio/bgm/12. Main Music 06.flac",
		"assets/audio/bgm/13. Main Music 07.flac",
		"assets/audio/bgm/14. Main Music 08.flac",
		"assets/audio/bgm/15. Main Music 09.flac",
		"assets/audio/bgm/16. Main Music 10.flac",
		"assets/audio/bgm/17. Main Music 11.flac",
		"assets/audio/bgm/18. Main Music 12.flac",
		"assets/audio/bgm/19. Main Music 13.flac"
	};
}

void AudioManager::updateBGM(WeatherType currentWeather) {
	const std::string nextTrack = weatherToTrackPath(currentWeather);
	if (isPlaying_ && currentWeather_ == currentWeather && currentTrackPath_ == nextTrack) {
		return;
	}

	currentTrack_.stop();
	isPlaying_ = false;
	currentWeather_ = currentWeather;

	for (const auto& candidate : buildTrackCandidates(currentWeather)) {
		if (std::filesystem::exists(candidate) && currentTrack_.openFromFile(candidate)) {
			currentTrack_.setLooping(true);
			currentTrack_.setVolume(45.f);
			currentTrack_.play();
			currentTrackPath_ = candidate;
			isPlaying_ = true;
			std::cout << "AudioManager: playing " << currentTrackPath_ << std::endl;
			return;
		}
	}

	std::cout << "AudioManager: no bgm track loaded" << std::endl;
}

void AudioManager::stop() {
	if (!isPlaying_) {
		return;
	}

	currentTrack_.stop();
	isPlaying_ = false;
	currentTrackPath_.clear();
	std::cout << "AudioManager: BGM stopped" << std::endl;
}
