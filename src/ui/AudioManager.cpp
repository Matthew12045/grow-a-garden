#include "AudioManager.h"

#include <algorithm>
#include <filesystem>
#include <iostream>

namespace {
std::string normalizeAssetRelativePath(std::string relativePath) {
	std::replace(relativePath.begin(), relativePath.end(), '\\', '/');
	return relativePath;
}

bool isSafeAssetRelativePath(const std::string& relativePath) {
	if (relativePath.empty() || relativePath.front() == '/' ||
		relativePath.find(':') != std::string::npos) {
		return false;
	}

	const std::filesystem::path path(relativePath);
	if (path.empty() || path.is_absolute() || path.has_root_name() || path.has_root_directory()) {
		return false;
	}

	for (const auto& part : path) {
		if (part == "..") {
			return false;
		}
	}

	return true;
}
}

AudioManager::AudioManager()
	: currentWeather_(WeatherType::SUMMER),
	  isPlaying_(false) {
}

std::string AudioManager::weatherToTrackPath(WeatherType weatherType) {
	switch (weatherType) {
		case WeatherType::SUMMER:
			return "assets/audio/bgm/06_main_music_00.flac";
		case WeatherType::RAIN:
			return "assets/audio/bgm/10_main_music_04.flac";
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

bool AudioManager::playSound(const std::string& relativePath) {
	std::string normalizedPath = normalizeAssetRelativePath(relativePath);
	if (!isSafeAssetRelativePath(normalizedPath)) {
		return false;
	}

	const std::filesystem::path assetPath = std::filesystem::path("assets") / normalizedPath;
	std::error_code errorCode;
	if (!std::filesystem::is_regular_file(assetPath, errorCode) || errorCode) {
		return false;
	}

	activeSounds_.erase(
		std::remove_if(activeSounds_.begin(), activeSounds_.end(), [](const sf::Sound& sound) {
			return sound.getStatus() == sf::SoundSource::Status::Stopped;
		}),
		activeSounds_.end());

	auto [bufferIt, inserted] = soundBuffers_.try_emplace(normalizedPath);
	if (inserted && !bufferIt->second.loadFromFile(assetPath)) {
		soundBuffers_.erase(bufferIt);
		return false;
	}

	activeSounds_.emplace_back(bufferIt->second);
	activeSounds_.back().play();
	return true;
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
