#include "Game.h"
#include <chrono>
#include <iostream>

Game::Game()
    : tickSystem_(1.0f), // 1 tick per second by default
      garden_(20, 20),
      weatherSystem_(),
      player_(),
      lastSaveTimestamp_(0) {
}

void Game::update(float deltaTime) {
    tickSystem_.update(deltaTime);
    // In a real implementation, we would check if a tick occurred and update other systems
}

void Game::autoSave() {
    saveGame();
}

void Game::saveGame() {
    auto now = std::chrono::system_clock::now();
    lastSaveTimestamp_ = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    std::cout << "Game saved. Timestamp: " << lastSaveTimestamp_ << std::endl;
}

void Game::loadGame() {
    // Load state from file (mocked for now)
    processOfflineProgress();
}

void Game::processOfflineProgress() {
    if (lastSaveTimestamp_ == 0) return; // No previous save

    auto now = std::chrono::system_clock::now();
    long currentTimestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

    long elapsedSeconds = currentTimestamp - lastSaveTimestamp_;
    if (elapsedSeconds > 0) {
        // Fast forward ticks based on elapsed time (assuming 1 tick per second)
        std::size_t ticksToForward = static_cast<std::size_t>(elapsedSeconds);
        tickSystem_.fastForward(ticksToForward);
        std::cout << "Processed offline progress. Fast forwarded " << ticksToForward << " ticks." << std::endl;
    }
}