#include "HarvestedItem.h"

#include <sstream>
#include <utility>

namespace {
const char* mutationTypeToString(MutationType type)
{
	switch (type) {
	case MutationType::WET:
		return "WET";
	case MutationType::SHOCKED:
		return "SHOCKED";
	case MutationType::FROZEN:
		return "FROZEN";
	case MutationType::CELESTIAL:
		return "CELESTIAL";
	default:
		return "UNKNOWN";
	}
}
} // namespace

HarvestedItem::HarvestedItem()
	: Item(0, "Harvested Item", "", 0.0),
	  mutations_(),
	  finalSellPrice_(0.0) {}

HarvestedItem::HarvestedItem(double finalSellPrice, std::vector<MutationType> mutations)
	: Item(0, "Harvested Item", "", finalSellPrice),
	  mutations_(std::move(mutations)),
	  finalSellPrice_(finalSellPrice) {}

std::unique_ptr<Item> HarvestedItem::clone() const
{
	return std::make_unique<HarvestedItem>(*this);
}

double HarvestedItem::getPrice() const
{
	return finalSellPrice_;
}

const std::vector<MutationType>& HarvestedItem::getMutationList() const
{
	return mutations_;
}

std::string HarvestedItem::getMutations() const
{
	if (mutations_.empty()) {
		return "NONE";
	}

	std::ostringstream oss;
	for (std::size_t i = 0; i < mutations_.size(); ++i) {
		if (i > 0) {
			oss << ", ";
		}
		oss << mutationTypeToString(mutations_[i]);
	}

	return oss.str();
}
