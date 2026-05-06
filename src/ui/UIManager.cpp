#include "UIManager.h"

#include <iomanip>
#include <iostream>
#include <sstream>

UIManager::UIManager()
	: language_(Language::ENGLISH) {
}

void UIManager::setLanguage(Language lang) {
	language_ = lang;
}

std::string UIManager::translate(const std::string& thai,
								 const std::string& english,
								 const std::string& chinese) const {
	switch (language_) {
		case Language::THAI:
			return thai;
		case Language::ENGLISH:
			return english;
		case Language::CHINESE:
			return chinese;
	}

	return english;
}

std::string UIManager::weatherToString(WeatherType weather) const {
	switch (weather) {
		case WeatherType::SUMMER:
			return translate("ฤดูร้อน", "Summer", "夏天");
		case WeatherType::RAIN:
			return translate("ฝน", "Rain", "下雨");
		case WeatherType::FROST:
			return translate("น้ำค้างแข็ง", "Frost", "霜冻");
		case WeatherType::THUNDER_STORM:
			return translate("พายุฟ้าคะนอง", "Thunder Storm", "雷暴");
		case WeatherType::METEOR_SHOWER:
			return translate("ฝนดาวตก", "Meteor Shower", "流星雨");
	}

	return translate("ไม่ทราบ", "Unknown", "未知");
}

std::string UIManager::mutationToString(MutationType mutation) const {
	switch (mutation) {
		case MutationType::WET:
			return translate("เปียก", "Wet", "潮湿");
		case MutationType::SHOCKED:
			return translate("ช็อต", "Shocked", "电击");
		case MutationType::FROZEN:
			return translate("แข็ง", "Frozen", "冰冻");
		case MutationType::CELESTIAL:
			return translate("สวรรค์", "Celestial", "天体");
	}

	return translate("ไม่ทราบ", "Unknown", "未知");
}

void UIManager::showInventory(const Inventory& inv) const {
	std::cout << translate("คลังของ", "Inventory", "背包") << '\n';

	const auto& items = inv.getItems();
	if (items.empty()) {
		std::cout << "  " << translate("ไม่มีไอเทม", "No items", "没有物品") << '\n';
		return;
	}

	for (const auto& item : items) {
		if (!item) {
			continue;
		}

		std::ostringstream line;
		line << "  - " << item->getName()
			 << " x" << inv.getQuantity(item->getName())
			 << " | $" << std::fixed << std::setprecision(2) << item->getPrice();
		std::cout << line.str() << '\n';
	}
}

void UIManager::showShop(const Shop& shop) const {
	std::cout << translate("ร้านค้า", "Shop", "商店") << '\n';

	const auto& items = shop.getAvailableItems();
	if (items.empty()) {
		std::cout << "  " << translate("ไม่มีสินค้า", "No items", "没有商品") << '\n';
		return;
	}

	for (const auto& item : items) {
		if (!item) {
			continue;
		}

		std::ostringstream line;
		line << "  - " << item->getName()
			 << " | " << item->getDescr()
			 << " | $" << std::fixed << std::setprecision(2) << item->getPrice();
		std::cout << line.str() << '\n';
	}
}

void UIManager::showWeather(WeatherType weather) const {
	std::cout << translate("สภาพอากาศ", "Weather", "天气") << ": "
			  << weatherToString(weather) << '\n';
}

void UIManager::showCropTimer(const Plant& plant) const {
	std::cout << translate("เวลาปลูก", "Crop timer", "作物计时") << ": "
			  << plant.getTimeToGrowth() << ' '
			  << translate("tick", "ticks", "刻") << '\n';
}

void UIManager::showMutations(const Plant& plant) const {
	std::cout << translate("การกลายพันธุ์", "Mutations", "变异") << '\n';

	const auto mutations = plant.getMutations();
	if (mutations.empty()) {
		std::cout << "  " << translate("ไม่มี", "None", "无") << '\n';
		return;
	}

	for (const auto mutation : mutations) {
		std::cout << "  - " << mutationToString(mutation) << '\n';
	}
}

void UIManager::showEventLog() const {
	std::cout << translate("บันทึกเหตุการณ์", "Event log", "事件日志") << '\n';
	std::cout << "  " << translate("ยังไม่มีเหตุการณ์", "No events yet", "暂无事件") << '\n';
}
