// Unit tests for UIManager and AudioManager
#include <gtest/gtest.h>
#include "../src/ui/UIManager.h"
#include "../src/ui/AudioManager.h"

TEST(UIManagerTest, TranslateLanguages) {
    UIManager ui;
    ui.setLanguage(Language::THAI);
    EXPECT_EQ(ui.translate("ไทย", "eng", "中"), "ไทย");
    ui.setLanguage(Language::ENGLISH);
    EXPECT_EQ(ui.translate("ไทย", "eng", "中"), "eng");
    ui.setLanguage(Language::CHINESE);
    EXPECT_EQ(ui.translate("ไทย", "eng", "中"), "中");
}

TEST(UIManagerTest, WeatherToString) {
    UIManager ui;
    ui.setLanguage(Language::ENGLISH);
    EXPECT_EQ(ui.weatherToString(WeatherType::SUMMER), "Summer");
    EXPECT_EQ(ui.weatherToString(WeatherType::RAIN), "Rain");
}

TEST(UIManagerTest, MutationToString) {
    UIManager ui;
    ui.setLanguage(Language::ENGLISH);
    EXPECT_EQ(ui.mutationToString(MutationType::WET), "Wet");
    EXPECT_EQ(ui.mutationToString(MutationType::FROZEN), "Frozen");
}

TEST(AudioManagerTest, WeatherToTrackPath) {
    EXPECT_NE(AudioManager::weatherToTrackPath(WeatherType::SUMMER).find("06_main_music_00"), std::string::npos);
    EXPECT_NE(AudioManager::weatherToTrackPath(WeatherType::METEOR_SHOWER).find("19_main_music_13"), std::string::npos);
}

TEST(AudioManagerTest, PlaySoundReturnsFalseForMissingAsset) {
    AudioManager audio;
    EXPECT_FALSE(audio.playSound("audio/sfx/not-real.wav"));
}

TEST(AudioManagerTest, PlaySoundReturnsFalseForInvalidPaths) {
    AudioManager audio;
    EXPECT_FALSE(audio.playSound(""));
    EXPECT_FALSE(audio.playSound("../audio/sfx/not-real.wav"));
    EXPECT_FALSE(audio.playSound("audio/../sfx/not-real.wav"));
    EXPECT_FALSE(audio.playSound("/audio/sfx/not-real.wav"));
    EXPECT_FALSE(audio.playSound("C:/audio/sfx/not-real.wav"));
}
