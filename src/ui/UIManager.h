#pragma once

#include "../entities/Plant.h"
#include "../systems/Inventory.h"
#include "../systems/Shop.h"
#include "../world/WeatherSystem.h"

#include <string>

// TODO: Wire UIManager into the active game screen flow as the shared
// text/console UI layer for inventory, shop, weather, crop, and event output.
enum class Language {
	THAI,
	ENGLISH,
	CHINESE
};

class UIManager {
private:
	Language language_;

public:
	std::string translate(const std::string& thai,
					  const std::string& english,
					  const std::string& chinese) const;
	std::string weatherToString(WeatherType weather) const;
	std::string mutationToString(MutationType mutation) const;
    
	UIManager();

	void showInventory(const Inventory& inv) const;
	void showShop(const Shop& shop) const;
	void showWeather(WeatherType weather) const;
	void showCropTimer(const Plant& plant) const;
	void showMutations(const Plant& plant) const;
	void showEventLog() const;
	void setLanguage(Language lang);
};
