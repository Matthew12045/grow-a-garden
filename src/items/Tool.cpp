#include "Tool.h"

#include <algorithm>
#include <utility>

Tool::Tool(std::size_t id,
		   std::string name,
		   std::string description,
		   double basePrice,
		   int durability)
	: Item(id, std::move(name), std::move(description), basePrice),
	  durability_(std::max(0, durability)),
	  maxDurability_(std::max(0, durability)) {}

double Tool::getPrice() const
{
	return basePrice_;
}

int Tool::getDurability() const
{
	return durability_;
}

int Tool::getMaxDurability() const
{
	return maxDurability_;
}

bool Tool::isBroken() const
{
	return durability_ <= 0;
}

void Tool::resetDurability()
{
	durability_ = maxDurability_;
}

void Tool::setDurability(int durability)
{
	durability_ = std::clamp(durability, 0, maxDurability_);
}

bool Tool::consumeDurability(int amount)
{
	if (amount <= 0 || isBroken()) {
		return false;
	}

	durability_ = std::max(0, durability_ - amount);
	return true;
}
