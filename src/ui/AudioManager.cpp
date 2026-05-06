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
			return "assets/audio/bgm/06_main_music_00.flac";
		case WeatherType::RAIN:
			return "assets/audio/bgm/09_main_music_03.flac";
		case WeatherType::FROST:
			return "assets/audio/bgm/12_main_music_06.flac";
		case WeatherType::THUNDER_STORM:
			return "assets/audio/bgm/16_main_music_10.flac";
		case WeatherType::METEOR_SHOWER:
			return "assets/audio/bgm/19_main_music_13.flac";
	}

	return "assets/audio/bgm/06_main_music_00.flac";
}

std::vector<std::string> AudioManager::buildTrackCandidates(WeatherType weatherType) {
	// ordered candidates with deduplication
	std::vector<std::string> all = {
		weatherToTrackPath(weatherType),
		"assets/audio/bgm/06_main_music_00.flac",
		"assets/audio/bgm/07_main_music_01.flac",
		"assets/audio/bgm/08_main_music_02.flac",
		"assets/audio/bgm/09_main_music_03.flac",
		"assets/audio/bgm/10_main_music_04.flac",
		"assets/audio/bgm/11_main_music_05.flac",
		"assets/audio/bgm/12_main_music_06.flac",
		"assets/audio/bgm/13_main_music_07.flac",
		"assets/audio/bgm/14_main_music_08.flac",
		"assets/audio/bgm/15_main_music_09.flac",
		"assets/audio/bgm/16_main_music_10.flac",
		"assets/audio/bgm/17_main_music_11.flac",
		"assets/audio/bgm/18_main_music_12.flac",
		"assets/audio/bgm/19_main_music_13.flac"
	};

	std::vector<std::string> uniq;
	for (const auto &p : all) {
		if (std::find(uniq.begin(), uniq.end(), p) == uniq.end()) {
			uniq.push_back(p);
		}
	}

	return uniq;
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
#ifndef NDEBUG
			std::cout << "AudioManager: playing " << currentTrackPath_ << std::endl;
#endif
			return;
		}
	}

#ifndef NDEBUG
	std::cout << "AudioManager: no bgm track loaded" << std::endl;
#endif
}

void AudioManager::stop() {
	if (!isPlaying_) {
		return;
	}

	currentTrack_.stop();
	isPlaying_ = false;
	currentTrackPath_.clear();
#ifndef NDEBUG
	std::cout << "AudioManager: BGM stopped" << std::endl;
#endif
}
